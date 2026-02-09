#ifndef POCKETOS_MCP23008_DRIVER_H
#define POCKETOS_MCP23008_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MCP23008_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// MCP23008 valid I2C addresses (A2, A1, A0 pins: 0x20-0x27)
#define MCP23008_ADDR_COUNT 8
const uint8_t MCP23008_VALID_ADDRESSES[MCP23008_ADDR_COUNT] = { 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 
};

// MCP23008 register addresses
#define MCP23008_REG_IODIR      0x00
#define MCP23008_REG_IPOL       0x01
#define MCP23008_REG_GPINTEN    0x02
#define MCP23008_REG_DEFVAL     0x03
#define MCP23008_REG_INTCON     0x04
#define MCP23008_REG_IOCON      0x05
#define MCP23008_REG_GPPU       0x06
#define MCP23008_REG_INTF       0x07
#define MCP23008_REG_INTCAP     0x08
#define MCP23008_REG_GPIO       0x09
#define MCP23008_REG_OLAT       0x0A

// MCP23008 Device Driver (8-bit GPIO expander)
class MCP23008Driver {
public:
    MCP23008Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Tier 0: Basic GPIO operations
    bool pinMode(uint8_t pin, uint8_t mode);
    bool digitalWrite(uint8_t pin, bool value);
    int digitalRead(uint8_t pin);
    bool writePort(uint8_t value);
    uint8_t readPort();
    
#if POCKETOS_MCP23008_ENABLE_CONFIGURATION
    // Tier 1: Advanced features
    bool setPullUp(uint8_t pin, bool enable);
    bool setPolarity(uint8_t pin, bool inverted);
    bool enableInterrupt(uint8_t pin, uint8_t mode);
    bool disableInterrupt(uint8_t pin);
    uint8_t getInterruptFlags();
    uint8_t getInterruptCapture();
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mcp23008"; }
    String getDriverTier() const { return POCKETOS_MCP23008_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = MCP23008_ADDR_COUNT;
        return MCP23008_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MCP23008_ADDR_COUNT; i++) {
            if (MCP23008_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MCP23008_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_MCP23008_ENABLE_LOGGING
    uint32_t operationCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_MCP23008_DRIVER_H
