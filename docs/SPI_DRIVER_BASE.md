# SPI Driver Base Infrastructure for PocketOS

This document describes the SPI driver base infrastructure added to PocketOS, following the same tiering system as the I2C drivers.

## Overview

The SPI driver base infrastructure provides a reusable foundation for creating SPI device drivers with:
- Automatic endpoint descriptor parsing
- Pin resource management via ResourceManager
- Multiple register access conventions (nRF24, MCP2515, Generic)
- Per-driver SPI configuration
- Tier 2 register access support

## File Locations

- `/src/pocketos/drivers/spi_driver_base.h` - SPI driver base class header
- `/src/pocketos/drivers/spi_driver_base.cpp` - SPI driver base class implementation
- `/src/pocketos/drivers/register_types.h` - Extended with BusType enum
- `/src/pocketos/core/device_registry.h` - Extended with IRegisterAccess interface
- `/src/pocketos/core/device_registry.cpp` - Updated to support both I2C and SPI register access

## Key Features

### 1. Endpoint Descriptor Parsing

The SPI driver base can parse endpoint descriptors in the format:
```
spi0:cs=5,dc=16,rst=17,irq=4,busy=27
```

Where:
- `spi0` - SPI bus ID (required)
- `cs` - Chip Select pin (required)
- `dc` - Data/Command pin (optional, for displays)
- `rst` or `reset` - Reset pin (optional)
- `irq` or `int` - Interrupt pin (optional)
- `busy` - Busy status pin (optional)

Example usage:
```cpp
SPIDriverBase driver;
driver.initFromEndpoint("spi0:cs=5,dc=16,rst=17");
```

### 2. Pin Resource Management

All pins specified in the endpoint descriptor are automatically claimed via ResourceManager:
- Prevents resource conflicts
- Tracks ownership
- Automatic release on deinit()

### 3. Register Access Conventions

Three register access conventions are supported:

#### Generic (Address + Data)
```cpp
setRegisterConvention(SPIRegisterConvention::GENERIC);
// Read: send address, read data
// Write: send address, write data
```

#### nRF24 Style (Modified Address)
```cpp
setRegisterConvention(SPIRegisterConvention::NRF24);
// Read: send (reg | 0x80), read data
// Write: send (reg & 0x7F), write data
```

#### MCP2515 Style (Command Byte)
```cpp
setRegisterConvention(SPIRegisterConvention::MCP2515);
// Read: send READ_CMD (0x03), send address, read data
// Write: send WRITE_CMD (0x02), send address, write data
```

### 4. Pin Control Helpers

The base class provides convenient helpers for controlling optional pins:

```cpp
// CS control (active low)
setCS(true);   // Assert CS
setCS(false);  // Release CS

// DC control (for displays)
// Most displays use LOW=command, HIGH=data convention
setDC(true);        // Command mode (DC=LOW)
setDC(false);       // Data mode (DC=HIGH)
// Or use explicit named methods:
setDCCommand();     // Command mode (DC=LOW)
setDCData();        // Data mode (DC=HIGH)

// Reset control (active low)
setRST(true);  // Assert reset
setRST(false); // Release reset

// Read busy status
bool busy = readBusy();
```

### 5. SPI Transaction Management

Low-level SPI operations with automatic transaction handling:

```cpp
// Simple transfer (in-place)
spiTransfer(data, len);

// Write only
spiWrite(data, len);

// Read only
spiRead(buffer, len);

// Write then read
spiWriteRead(write_data, write_len, read_buffer, read_len);
```

### 6. Register Access (Tier 2)

Virtual methods for register access that drivers can override:

```cpp
// Read register(s)
bool regRead(uint16_t reg, uint8_t* buf, size_t len);

// Write register(s)
bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);

// Get register map (Tier 2 drivers)
const RegisterDesc* registers(size_t& count) const;

// Find register by name
const RegisterDesc* findRegisterByName(const String& name) const;
```

## Creating an SPI Driver

### Example: nRF24L01 Driver

```cpp
#include "spi_driver_base.h"

class NRF24Driver : public SPIDriverBase, public IDriver, public IRegisterAccess {
public:
    NRF24Driver() {
        setRegisterConvention(SPIRegisterConvention::NRF24);
    }
    
    // IDriver interface
    bool init() override {
        // Parse endpoint from constructor or parameter
        if (!initFromEndpoint("spi0:cs=5,irq=4")) {
            return false;
        }
        
        // Device-specific initialization
        uint8_t config = 0x0E;  // Power up, enable CRC
        regWrite(0x00, &config, 1);
        
        return true;
    }
    
    void update() override {
        // Periodic update logic
    }
    
    bool setParam(const String& name, const String& value) override {
        // Parameter handling
        return false;
    }
    
    String getParam(const String& name) override {
        return "";
    }
    
    CapabilitySchema getSchema() override {
        // Return capability schema
        return CapabilitySchema();
    }
    
    // IRegisterAccess interface (Tier 2)
    BusType getBusType() const override {
        return BusType::SPI;
    }
    
    const RegisterDesc* registers(size_t& count) const override {
        static const RegisterDesc NRF24_REGISTERS[] = {
            RegisterDesc(0x00, "CONFIG", 1, RegisterAccess::RW, 0x08),
            RegisterDesc(0x01, "EN_AA", 1, RegisterAccess::RW, 0x3F),
            RegisterDesc(0x02, "EN_RXADDR", 1, RegisterAccess::RW, 0x03),
            // ... more registers
        };
        count = sizeof(NRF24_REGISTERS) / sizeof(RegisterDesc);
        return NRF24_REGISTERS;
    }
};
```

### Example: MCP2515 CAN Controller

```cpp
class MCP2515Driver : public SPIDriverBase, public IDriver {
public:
    MCP2515Driver() {
        setRegisterConvention(SPIRegisterConvention::MCP2515);
    }
    
    bool init() override {
        if (!initFromEndpoint("spi0:cs=10,int=2")) {
            return false;
        }
        
        // Send RESET command
        uint8_t reset = 0xC0;
        spiWrite(&reset, 1);
        delay(10);
        
        // Configure device
        uint8_t mode = 0x80;  // Configuration mode
        regWrite(0x0F, &mode, 1);
        
        return true;
    }
    
    // ... implement IDriver interface
};
```

## DeviceRegistry Integration

The DeviceRegistry has been extended to support both I2C and SPI drivers with register access:

### IRegisterAccess Interface

Drivers that support Tier 2 register access should implement this interface:

```cpp
class IRegisterAccess {
public:
    virtual const RegisterDesc* registers(size_t& count) const = 0;
    virtual bool regRead(uint16_t reg, uint8_t* buf, size_t len) = 0;
    virtual bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) = 0;
    virtual BusType getBusType() const = 0;
};
```

### Automatic Bus Type Detection

The DeviceRegistry automatically detects the bus type and routes register access appropriately:

```cpp
// Works for both I2C and SPI drivers
bool DeviceRegistry::deviceRegRead(int deviceId, uint16_t reg, 
                                   uint8_t* buf, size_t len);
bool DeviceRegistry::deviceRegWrite(int deviceId, uint16_t reg, 
                                    const uint8_t* buf, size_t len);
String DeviceRegistry::getDeviceRegisters(int deviceId);
```

## Tiering System

The SPI driver base follows the same tiering system as I2C drivers:

### Tier 0 (Basic)
- Basic SPI communication
- Essential device functionality
- Minimal code size

### Tier 1 (Robust)
- Tier 0 features
- Configuration options
- Advanced diagnostics
- Error handling

### Tier 2 (Complete)
- Tier 1 features
- Full register access via `regRead/regWrite`
- Complete register map via `registers()`
- Register access via DeviceRegistry

Example configuration in driver_config.h:
```cpp
// Global tier setting
#define POCKETOS_DRIVER_TIER 1  // 0, 1, or 2

// Per-driver tier override
#define POCKETOS_NRF24_TIER 2  // Enable full register access
```

## Register Types

The `register_types.h` header has been extended with:

### BusType Enum
```cpp
enum class BusType {
    I2C = 0,
    SPI = 1,
    UNKNOWN = 255
};
```

### Bus Type Utilities
```cpp
const char* RegisterUtils::busTypeToString(BusType type);
```

## Usage Examples

### Binding an SPI Device

```cpp
// Register endpoint
EndpointRegistry::registerEndpoint("spi0:cs=5,dc=16", 
                                  EndpointType::SPI_DEVICE, 5);

// Bind driver
int deviceId = DeviceRegistry::bindDevice("nrf24", "spi0:cs=5,dc=16");

// Access registers (Tier 2)
if (DeviceRegistry::deviceSupportsRegisters(deviceId)) {
    uint8_t value;
    DeviceRegistry::deviceRegRead(deviceId, 0x00, &value, 1);
    
    value = 0x0E;
    DeviceRegistry::deviceRegWrite(deviceId, 0x00, &value, 1);
}
```

### Different Pin Configurations

```cpp
// Minimal: CS only
"spi0:cs=5"

// With DC (displays)
"spi0:cs=5,dc=16"

// With RST (wireless modules)
"spi0:cs=5,rst=17"

// With IRQ and BUSY (nRF24, LoRa)
"spi0:cs=5,irq=4,busy=27"

// Full configuration
"spi0:cs=5,dc=16,rst=17,irq=4,busy=27"
```

## Testing

Example test code for verifying the SPI driver base:

```cpp
void testSPIDriverBase() {
    // Create driver
    SPIDriverBase driver;
    
    // Test endpoint parsing
    assert(driver.initFromEndpoint("spi0:cs=5,dc=16,rst=17"));
    assert(driver.isInitialized());
    
    // Test pin configuration
    const SPIPinConfig& pins = driver.getPinConfig();
    assert(pins.cs == 5);
    assert(pins.dc == 16);
    assert(pins.rst == 17);
    
    // Test register conventions
    driver.setRegisterConvention(SPIRegisterConvention::NRF24);
    assert(driver.getRegisterConvention() == SPIRegisterConvention::NRF24);
    
    // Test register access
    uint8_t data = 0x55;
    assert(driver.regWrite(0x00, &data, 1));
    assert(driver.regRead(0x00, &data, 1));
    
    // Cleanup
    driver.deinit();
    assert(!driver.isInitialized());
}
```

## Benefits

1. **Consistency**: Same patterns as I2C drivers
2. **Reusability**: Common SPI operations in base class
3. **Flexibility**: Multiple register conventions supported
4. **Safety**: Automatic pin resource management
5. **Maintainability**: Single place for SPI logic
6. **Extensibility**: Easy to add new conventions
7. **Integration**: Works seamlessly with DeviceRegistry

## Future Enhancements

Potential future additions:
- DMA support for high-speed transfers
- Multi-byte register address support
- Custom register width handling
- Interrupt-driven SPI operations
- SPI transaction queuing
- Additional register conventions (MAX31855, BME280, etc.)
