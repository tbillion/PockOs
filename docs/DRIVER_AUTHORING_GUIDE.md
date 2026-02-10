# PocketOS Driver Authoring Guide

## Overview

This guide describes how to create compliant drivers for PocketOS that work with the tiered driver system and register access framework.

## Driver Tiering System

PocketOS uses a three-tier driver architecture that allows compile-time optimization:

### Tier 0: Basic Functionality
- **Purpose**: Minimum useful features
- **Code Size**: ~30-40% of full driver
- **Features**:
  - Core read/write operations only
  - Basic I2C/SPI communication
  - Essential sensor data reading
  - Minimal error checking

### Tier 1: Robust Features
- **Purpose**: Production-ready with configuration (DEFAULT)
- **Code Size**: ~70-80% of full driver
- **Features**:
  - All Tier 0 features
  - Full error handling with logging
  - Configuration options
  - Calibration and advanced modes
  - Health diagnostics

### Tier 2: Complete + Register Access
- **Purpose**: Complete driver with exhaustive register access
- **Code Size**: 100% of driver
- **Features**:
  - All Tier 1 features
  - Complete register map from datasheet
  - Register read/write via CLI
  - Full device introspection
  - Required for device identification logic

## Required Driver Interface

### Address Enumeration (All Tiers)

Every I2C driver MUST declare all valid 7-bit addresses it can operate on:

```cpp
// In header file
#define MYDRIVER_ADDR_COUNT 2
const uint8_t MYDRIVER_VALID_ADDRESSES[MYDRIVER_ADDR_COUNT] = { 0x44, 0x45 };

class MyDriver {
public:
    // Static method to get valid addresses
    static const uint8_t* validAddresses(size_t& count) {
        count = MYDRIVER_ADDR_COUNT;
        return MYDRIVER_VALID_ADDRESSES;
    }
    
    // Check if address is supported
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MYDRIVER_ADDR_COUNT; i++) {
            if (MYDRIVER_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
};
```

### Tier 2: Register Access (Required)

For Tier 2 drivers, you must expose the complete register map:

```cpp
#include "register_types.h"

#if POCKETOS_MYDRIVER_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
```

## Register Map Definition

### Register Descriptor Format

```cpp
RegisterDesc(
    uint16_t addr,         // Register address (8-bit or 16-bit)
    const char* name,      // CLI-friendly name (uppercase, underscores)
    uint8_t width,         // Width in bytes (1, 2, 3, or 4)
    RegisterAccess access, // RO, WO, RW, or RC (read-clear)
    uint32_t reset         // Reset value (if known, 0 if unknown)
)
```

### Complete Register Table Example (BME280)

```cpp
#if POCKETOS_BME280_ENABLE_REGISTER_ACCESS
static const RegisterDesc BME280_REGISTERS[] = {
    // Calibration registers (Read-only)
    RegisterDesc(0x88, "DIG_T1_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x89, "DIG_T1_MSB", 1, RegisterAccess::RO, 0x00),
    // ... more calibration regs ...
    
    // Chip ID (Read-only)
    RegisterDesc(0xD0, "CHIP_ID", 1, RegisterAccess::RO, 0x60),
    
    // Reset register (Write-only)
    RegisterDesc(0xE0, "RESET", 1, RegisterAccess::WO, 0x00),
    
    // Control registers (Read-write)
    RegisterDesc(0xF2, "CTRL_HUM", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xF3, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xF4, "CTRL_MEAS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xF5, "CONFIG", 1, RegisterAccess::RW, 0x00),
    
    // Data registers (Read-only)
    RegisterDesc(0xF7, "PRESS_MSB", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0xF8, "PRESS_LSB", 1, RegisterAccess::RO, 0x00),
    // ... more data regs ...
};

#define BME280_REGISTER_COUNT (sizeof(BME280_REGISTERS) / sizeof(RegisterDesc))
```

### Implementing Register Access

```cpp
const RegisterDesc* BME280Driver::registers(size_t& count) const {
    count = BME280_REGISTER_COUNT;
    return BME280_REGISTERS;
}

bool BME280Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    // Validate register exists and is readable
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        BME280_REGISTERS, BME280_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    // Perform I2C read
    return readRegister((uint8_t)reg, buf);
}

bool BME280Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    // Validate register exists and is writable
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        BME280_REGISTERS, BME280_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    // Perform I2C write
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* BME280Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(BME280_REGISTERS, BME280_REGISTER_COUNT, name);
}
```

## driver_config.h Integration

### Define Tier Override

```cpp
// MyDriver Tier (Description)
#ifndef POCKETOS_DRIVER_TIER_MYDRIVER
#define POCKETOS_DRIVER_TIER_MYDRIVER POCKETOS_DRIVER_TIER
#endif
```

### Define Feature Flags

```cpp
// MyDriver Feature Flags
// Tier 0: Basic read functionality
#if POCKETOS_DRIVER_TIER_MYDRIVER >= POCKETOS_TIER_0
#define POCKETOS_MYDRIVER_ENABLE_BASIC_READ 1
#else
#define POCKETOS_MYDRIVER_ENABLE_BASIC_READ 0
#endif

// Tier 1: Robust features
#if POCKETOS_DRIVER_TIER_MYDRIVER >= POCKETOS_TIER_1
#define POCKETOS_MYDRIVER_ENABLE_ERROR_HANDLING 1
#define POCKETOS_MYDRIVER_ENABLE_LOGGING 1
#define POCKETOS_MYDRIVER_ENABLE_CONFIGURATION 1
#else
#define POCKETOS_MYDRIVER_ENABLE_ERROR_HANDLING 0
#define POCKETOS_MYDRIVER_ENABLE_LOGGING 0
#define POCKETOS_MYDRIVER_ENABLE_CONFIGURATION 0
#endif

// Tier 2: Complete + exhaustive register access
#if POCKETOS_DRIVER_TIER_MYDRIVER >= POCKETOS_TIER_2
#define POCKETOS_MYDRIVER_ENABLE_REGISTER_ACCESS 1
#else
#define POCKETOS_MYDRIVER_ENABLE_REGISTER_ACCESS 0
#endif
```

### Add Tier Name Macro

```cpp
#define POCKETOS_MYDRIVER_TIER_NAME POCKETOS_TIER_NAME(POCKETOS_DRIVER_TIER_MYDRIVER)
```

### Add Validation

```cpp
#if POCKETOS_DRIVER_TIER_MYDRIVER < 0 || POCKETOS_DRIVER_TIER_MYDRIVER > 2
#error "POCKETOS_DRIVER_TIER_MYDRIVER must be 0, 1, or 2"
#endif
```

## Driver File Structure

### Required Files

```
src/pocketos/drivers/
├── mydriver.h           # Header with class definition
├── mydriver.cpp         # Implementation
└── register_types.h     # Shared (already exists)
```

### Header Template

```cpp
#ifndef POCKETOS_MYDRIVER_H
#define POCKETOS_MYDRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MYDRIVER_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// Valid I2C addresses
#define MYDRIVER_ADDR_COUNT 2
const uint8_t MYDRIVER_VALID_ADDRESSES[MYDRIVER_ADDR_COUNT] = { 0x40, 0x41 };

class MyDriver {
public:
    MyDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mydriver"; }
    String getDriverTier() const { return POCKETOS_MYDRIVER_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count);
    static bool supportsAddress(uint8_t addr);
    
#if POCKETOS_MYDRIVER_ENABLE_REGISTER_ACCESS
    // Tier 2: Register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
};

} // namespace PocketOS

#endif
```

## CLI Usage Examples

Once your driver is integrated:

```bash
# Bind device
> bind mydriver i2c0:0x40
OK device_id=1

# List all registers (Tier 2 only)
> reg list 1
0x00 REG_CTRL 1 RW 0x00
0x01 REG_STATUS 1 RO 0x00
0x02 REG_DATA 1 RO 0x00

# Read register by address
> reg read 1 0x00
register=0x00
value=27
length=1

# Read register by name
> reg read 1 REG_CTRL
register=0x00
value=27
length=1

# Write register
> reg write 1 0x00 0x03
register=0x00
value=0x03
```

## Best Practices

1. **Start with Tier 0**: Implement basic functionality first, then add tiers
2. **Complete Register Maps**: Include ALL registers from the datasheet for Tier 2
3. **Accurate Reset Values**: Specify correct reset values from datasheet
4. **Access Type Accuracy**: Use correct access types (RO/WO/RW/RC)
5. **Register Name Convention**: Use UPPERCASE_WITH_UNDERSCORES matching datasheet
6. **Validation**: Always validate register access in regRead/regWrite
7. **CRC/Checksums**: Implement in Tier 1+ for devices that support it
8. **Error Handling**: Log errors appropriately based on tier

## Testing Checklist

- [ ] Driver compiles at all tiers (0, 1, 2)
- [ ] validAddresses() returns all supported addresses
- [ ] supportsAddress() correctly validates addresses
- [ ] Tier 0: Basic read operations work
- [ ] Tier 1: Error handling and configuration work
- [ ] Tier 2: Register list returns complete map
- [ ] Tier 2: Register read works for all readable registers
- [ ] Tier 2: Register write works for all writable registers
- [ ] Tier 2: Read-only registers reject writes
- [ ] Tier 2: Write-only registers reject reads
- [ ] Schema includes tier name
- [ ] Code size reduces appropriately at lower tiers

## Common Patterns

### SPI Devices
- Valid addresses might be CS pin numbers
- Still use address enumeration pattern
- Register access works the same way

### Multi-Chip Devices
- Example: LSM9DS1 (accel/gyro + mag)
- Create separate drivers or subdevices
- Each declares its own valid addresses

### Variable Address Devices
- Example: VL53L0X (programmable address)
- List default address
- Document address programming in schema

## Integration with DeviceRegistry

To support register access in DeviceRegistry, add your driver:

```cpp
// In device_registry.cpp

#include "../drivers/mydriver.h"

// In getDeviceRegisters():
if (dev.driverId == "mydriver") {
    MyDriver* drv = static_cast<MyDriver*>(dev.driver);
    if (drv) {
#if POCKETOS_MYDRIVER_ENABLE_REGISTER_ACCESS
        size_t count;
        const RegisterDesc* regs = drv->registers(count);
        // ... format and return register list
#endif
    }
}

// Similar for deviceRegRead() and deviceRegWrite()
```

## References

- `src/pocketos/drivers/bme280_driver.*` - Complete Tier 2 example
- `src/pocketos/drivers/sht31_driver.*` - Tier 1 example
- `src/pocketos/drivers/register_types.h` - Register descriptor types
- `src/pocketos/driver_config.h` - Tier configuration system
- `docs/DRIVER_REG_ACCESS.md` - CLI usage examples
