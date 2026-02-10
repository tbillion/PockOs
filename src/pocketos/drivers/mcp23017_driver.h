#ifndef POCKETOS_MCP23017_DRIVER_H
#define POCKETOS_MCP23017_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MCP23017_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// MCP23017 valid I2C addresses (A2, A1, A0 pins: 0x20-0x27)
#define MCP23017_ADDR_COUNT 8
const uint8_t MCP23017_VALID_ADDRESSES[MCP23017_ADDR_COUNT] = { 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 
};

// MCP23017 register addresses (IOCON.BANK = 0)
#define MCP23017_REG_IODIRA     0x00
#define MCP23017_REG_IODIRB     0x01
#define MCP23017_REG_IPOLA      0x02
#define MCP23017_REG_IPOLB      0x03
#define MCP23017_REG_GPINTENA   0x04
#define MCP23017_REG_GPINTENB   0x05
#define MCP23017_REG_DEFVALA    0x06
#define MCP23017_REG_DEFVALB    0x07
#define MCP23017_REG_INTCONA    0x08
#define MCP23017_REG_INTCONB    0x09
#define MCP23017_REG_IOCON      0x0A
#define MCP23017_REG_GPPUA      0x0C
#define MCP23017_REG_GPPUB      0x0D
#define MCP23017_REG_INTFA      0x0E
#define MCP23017_REG_INTFB      0x0F
#define MCP23017_REG_INTCAPA    0x10
#define MCP23017_REG_INTCAPB    0x11
#define MCP23017_REG_GPIOA      0x12
#define MCP23017_REG_GPIOB      0x13
#define MCP23017_REG_OLATA      0x14
#define MCP23017_REG_OLATB      0x15

// MCP23017 Device Driver (16-bit GPIO expander)
class MCP23017Driver {
public:
    MCP23017Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Tier 0: Basic GPIO operations
    bool pinMode(uint8_t pin, uint8_t mode);
    bool digitalWrite(uint8_t pin, bool value);
    int digitalRead(uint8_t pin);
    bool writePort(uint16_t value);
    uint16_t readPort();
    bool writePortA(uint8_t value);
    bool writePortB(uint8_t value);
    uint8_t readPortA();
    uint8_t readPortB();
    
#if POCKETOS_MCP23017_ENABLE_CONFIGURATION
    // Tier 1: Advanced features
    bool setPullUp(uint8_t pin, bool enable);
    bool setPolarity(uint8_t pin, bool inverted);
    bool enableInterrupt(uint8_t pin, uint8_t mode);
    bool disableInterrupt(uint8_t pin);
    uint16_t getInterruptFlags();
    uint16_t getInterruptCapture();
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mcp23017"; }
    String getDriverTier() const { return POCKETOS_MCP23017_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = MCP23017_ADDR_COUNT;
        return MCP23017_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MCP23017_ADDR_COUNT; i++) {
            if (MCP23017_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MCP23017_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_MCP23017_ENABLE_LOGGING
    uint32_t operationCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    
    // Helper to get port registers
    uint8_t getPortReg(uint8_t pin, uint8_t regA, uint8_t regB);
};

} // namespace PocketOS

#endif // POCKETOS_MCP23017_DRIVER_H
