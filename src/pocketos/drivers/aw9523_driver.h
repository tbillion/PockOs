#ifndef POCKETOS_AW9523_DRIVER_H
#define POCKETOS_AW9523_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_AW9523_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// AW9523 valid I2C addresses (AD pin: 0x58-0x5B)
#define AW9523_ADDR_COUNT 4
const uint8_t AW9523_VALID_ADDRESSES[AW9523_ADDR_COUNT] = { 
    0x58, 0x59, 0x5A, 0x5B 
};

// AW9523 register addresses
#define AW9523_REG_INPUT0       0x00
#define AW9523_REG_INPUT1       0x01
#define AW9523_REG_OUTPUT0      0x02
#define AW9523_REG_OUTPUT1      0x03
#define AW9523_REG_CONFIG0      0x04
#define AW9523_REG_CONFIG1      0x05
#define AW9523_REG_INT0         0x06
#define AW9523_REG_INT1         0x07
#define AW9523_REG_ID           0x10
#define AW9523_REG_CTL          0x11
#define AW9523_REG_LED_MODE0    0x12
#define AW9523_REG_LED_MODE1    0x13
#define AW9523_REG_DIM0         0x20
#define AW9523_REG_DIM15        0x2F
#define AW9523_REG_SWRST        0x7F

// AW9523 Device Driver (16-ch GPIO + LED driver)
class AW9523Driver {
public:
    AW9523Driver();
    
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
    
#if POCKETOS_AW9523_ENABLE_CONFIGURATION
    // Tier 1: Advanced features
    bool setLedMode(uint8_t pin, bool ledMode);
    bool setLedBrightness(uint8_t pin, uint8_t brightness);
    bool enableInterrupt(uint8_t pin);
    bool disableInterrupt(uint8_t pin);
    uint16_t getInterruptStatus();
    bool softReset();
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "aw9523"; }
    String getDriverTier() const { return POCKETOS_AW9523_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = AW9523_ADDR_COUNT;
        return AW9523_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < AW9523_ADDR_COUNT; i++) {
            if (AW9523_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_AW9523_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_AW9523_ENABLE_LOGGING
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

#endif // POCKETOS_AW9523_DRIVER_H
