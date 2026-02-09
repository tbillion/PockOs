// Example SPI Driver using SPIDriverBase
// This demonstrates how to create an SPI driver for a device like the nRF24L01

#include "spi_driver_base.h"
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// Example: nRF24L01 driver using SPIDriverBase
class NRF24DriverExample : public SPIDriverBase {
public:
    NRF24DriverExample() {
        // Set register convention to nRF24 style (reg|0x80 for read)
        setRegisterConvention(SPIRegisterConvention::NRF24);
    }
    
    bool init(const String& endpoint) {
        // Initialize SPI base (parses endpoint, claims pins)
        if (!initFromEndpoint(endpoint)) {
            return false;
        }
        
        // Configure bus speed and mode
        SPIBusConfig config = getBusConfig();
        config.speed_hz = 8000000;  // 8 MHz
        config.mode = SPI_MODE0;
        
        // Perform hardware reset if RST pin available
        if (getPinConfig().rst >= 0) {
            setRST(true);   // Assert reset
            delay(10);
            setRST(false);  // Release reset
            delay(10);
        }
        
        // Read chip config register to verify communication
        uint8_t config_val;
        if (!regRead(0x00, &config_val, 1)) {
            return false;
        }
        
        return true;
    }
    
    // Override regRead if needed for special handling
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override {
        // Use base class implementation (handles nRF24 convention)
        return SPIDriverBase::regRead(reg, buf, len);
    }
    
    // Override regWrite if needed for special handling
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override {
        // Use base class implementation (handles nRF24 convention)
        return SPIDriverBase::regWrite(reg, buf, len);
    }
};

// Example: MCP2515 CAN controller driver
class MCP2515DriverExample : public SPIDriverBase {
public:
    MCP2515DriverExample() {
        // Set register convention to MCP2515 style (separate READ/WRITE commands)
        setRegisterConvention(SPIRegisterConvention::MCP2515);
    }
    
    bool init(const String& endpoint) {
        // Initialize SPI base
        if (!initFromEndpoint(endpoint)) {
            return false;
        }
        
        // Configure bus speed
        SPIBusConfig config = getBusConfig();
        config.speed_hz = 10000000;  // 10 MHz
        config.mode = SPI_MODE0;
        
        // Send RESET command (MCP2515 specific)
        uint8_t reset_cmd = 0xC0;
        spiWrite(&reset_cmd, 1);
        delay(10);
        
        return true;
    }
};

// Example: Generic SPI device (like display controller)
class GenericSPIDriverExample : public SPIDriverBase {
public:
    GenericSPIDriverExample() {
        // Use generic convention (address then data)
        setRegisterConvention(SPIRegisterConvention::GENERIC);
    }
    
    bool init(const String& endpoint) {
        // Initialize SPI base
        if (!initFromEndpoint(endpoint)) {
            return false;
        }
        
        // Configure for display-like devices
        SPIBusConfig config = getBusConfig();
        config.speed_hz = 20000000;  // 20 MHz
        config.mode = SPI_MODE0;
        
        // Use DC pin for command/data selection
        if (getPinConfig().dc >= 0) {
            // Send command using explicit method
            setDCCommand();  // Command mode (DC=LOW)
            uint8_t cmd = 0x01;
            spiWrite(&cmd, 1);
            
            // Send data using explicit method
            setDCData();  // Data mode (DC=HIGH)
            uint8_t data[] = {0x00, 0x01, 0x02};
            spiWrite(data, 3);
        }
        
        return true;
    }
};

} // namespace PocketOS
