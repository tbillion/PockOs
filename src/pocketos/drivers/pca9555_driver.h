#ifndef POCKETOS_PCA9555_DRIVER_H
#define POCKETOS_PCA9555_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_PCA9555_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// PCA9555 valid I2C addresses (A2, A1, A0 pins: 0x20-0x27)
#define PCA9555_ADDR_COUNT 8
const uint8_t PCA9555_VALID_ADDRESSES[PCA9555_ADDR_COUNT] = { 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 
};

// PCA9555 register addresses
#define PCA9555_REG_INPUT0      0x00
#define PCA9555_REG_INPUT1      0x01
#define PCA9555_REG_OUTPUT0     0x02
#define PCA9555_REG_OUTPUT1     0x03
#define PCA9555_REG_POLARITY0   0x04
#define PCA9555_REG_POLARITY1   0x05
#define PCA9555_REG_CONFIG0     0x06
#define PCA9555_REG_CONFIG1     0x07

// PCA9555 Device Driver (16-bit I/O expander)
class PCA9555Driver {
public:
    PCA9555Driver();
    
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
    bool writePort0(uint8_t value);
    bool writePort1(uint8_t value);
    uint8_t readPort0();
    uint8_t readPort1();
    
#if POCKETOS_PCA9555_ENABLE_CONFIGURATION
    // Tier 1: Advanced features
    bool setPolarity(uint8_t pin, bool inverted);
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pca9555"; }
    String getDriverTier() const { return POCKETOS_PCA9555_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = PCA9555_ADDR_COUNT;
        return PCA9555_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < PCA9555_ADDR_COUNT; i++) {
            if (PCA9555_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_PCA9555_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_PCA9555_ENABLE_LOGGING
    uint32_t operationCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    
    // Helper to get port registers
    uint8_t getPortReg(uint8_t pin, uint8_t reg0, uint8_t reg1);
};

} // namespace PocketOS

#endif // POCKETOS_PCA9555_DRIVER_H
