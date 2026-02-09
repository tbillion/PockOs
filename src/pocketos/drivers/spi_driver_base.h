#ifndef POCKETOS_SPI_DRIVER_BASE_H
#define POCKETOS_SPI_DRIVER_BASE_H

#include <Arduino.h>
#include <SPI.h>
#include "../driver_config.h"
#include "register_types.h"

namespace PocketOS {

// SPI register access conventions
enum class SPIRegisterConvention {
    GENERIC = 0,     // Address byte(s), then data
    NRF24 = 1,       // reg|0x80 for read, reg&0x7F for write
    MCP2515 = 2      // Separate READ/WRITE command bytes
};

// SPI pin assignment structure
struct SPIPinConfig {
    int8_t cs;      // Chip select (required)
    int8_t dc;      // Data/command (optional, -1 if unused)
    int8_t rst;     // Reset (optional, -1 if unused)
    int8_t irq;     // Interrupt (optional, -1 if unused)
    int8_t busy;    // Busy (optional, -1 if unused)
    
    SPIPinConfig() : cs(-1), dc(-1), rst(-1), irq(-1), busy(-1) {}
};

// SPI bus configuration
struct SPIBusConfig {
    uint8_t bus_id;
    uint32_t speed_hz;
    uint8_t mode;  // SPI mode (0-3)
    uint8_t bit_order;  // MSBFIRST or LSBFIRST
    
    SPIBusConfig() : bus_id(0), speed_hz(1000000), mode(SPI_MODE0), bit_order(MSBFIRST) {}
};

// SPI Driver Base Class
class SPIDriverBase {
public:
    SPIDriverBase();
    virtual ~SPIDriverBase();
    
    // Parse and initialize from endpoint descriptor
    // Format: "spi0:cs=5,dc=16,rst=17,irq=4,busy=27"
    bool initFromEndpoint(const String& endpoint);
    
    // Deinitialize and release resources
    void deinit();
    
    // Check if initialized
    bool isInitialized() const { return initialized_; }
    
    // Get pin configuration
    const SPIPinConfig& getPinConfig() const { return pins_; }
    const SPIBusConfig& getBusConfig() const { return bus_config_; }
    
    // Pin control helpers
    void setCS(bool active);  // CS control (active low by default)
    void setRST(bool active);  // RST control (active low by default)
    bool readBusy() const;     // Read BUSY pin
    
    // DC pin control (for displays)
    // Note: Most displays use LOW=command, HIGH=data convention
    void setDC(bool isCommand);  // DC control (true=command/LOW, false=data/HIGH)
    void setDCCommand();         // Set DC to command mode (LOW)
    void setDCData();            // Set DC to data mode (HIGH)
    
    // Register access convention configuration
    void setRegisterConvention(SPIRegisterConvention convention) { reg_convention_ = convention; }
    SPIRegisterConvention getRegisterConvention() const { return reg_convention_; }
    
    // Virtual methods for register access (to be overridden by drivers)
    virtual bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    virtual bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    
    // Register map access (Tier 2 drivers should override)
    virtual const RegisterDesc* registers(size_t& count) const { count = 0; return nullptr; }
    virtual const RegisterDesc* findRegisterByName(const String& name) const;
    
protected:
    // Low-level SPI transaction helpers
    bool spiTransfer(uint8_t* data, size_t len);
    bool spiWrite(const uint8_t* data, size_t len);
    bool spiRead(uint8_t* data, size_t len);
    bool spiWriteRead(const uint8_t* write_data, size_t write_len, uint8_t* read_data, size_t read_len);
    
    // Transaction management
    void beginTransaction();
    void endTransaction();
    
    // Helper for register access based on convention
    uint8_t prepareReadCommand(uint8_t reg);
    uint8_t prepareWriteCommand(uint8_t reg);
    
private:
    bool initialized_;
    SPIPinConfig pins_;
    SPIBusConfig bus_config_;
    SPIRegisterConvention reg_convention_;
    String owner_id_;  // For resource manager
    
    // Parse endpoint descriptor
    bool parseEndpoint(const String& endpoint);
    
    // Claim/release pins via ResourceManager
    bool claimPins();
    void releasePins();
};

// Helper class for register convention utilities
class SPIRegisterUtils {
public:
    // Convert register address for read operation
    static uint8_t toReadAddr(uint8_t reg, SPIRegisterConvention convention) {
        switch (convention) {
            case SPIRegisterConvention::NRF24:
                return reg | 0x80;
            case SPIRegisterConvention::MCP2515:
                return 0x03;  // READ command
            case SPIRegisterConvention::GENERIC:
            default:
                return reg;
        }
    }
    
    // Convert register address for write operation
    static uint8_t toWriteAddr(uint8_t reg, SPIRegisterConvention convention) {
        switch (convention) {
            case SPIRegisterConvention::NRF24:
                return reg & 0x7F;
            case SPIRegisterConvention::MCP2515:
                return 0x02;  // WRITE command
            case SPIRegisterConvention::GENERIC:
            default:
                return reg;
        }
    }
    
    // Check if convention requires separate command byte
    static bool requiresCommandByte(SPIRegisterConvention convention) {
        return convention == SPIRegisterConvention::MCP2515;
    }
};

} // namespace PocketOS

#endif // POCKETOS_SPI_DRIVER_BASE_H
