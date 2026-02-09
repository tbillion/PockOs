#ifndef POCKETOS_PCAL6416A_DRIVER_H
#define POCKETOS_PCAL6416A_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_PCAL6416A_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// PCAL6416A valid I2C addresses (A2, A1, A0 pins: 0x20-0x27)
#define PCAL6416A_ADDR_COUNT 8
const uint8_t PCAL6416A_VALID_ADDRESSES[PCAL6416A_ADDR_COUNT] = { 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 
};

// PCAL6416A register addresses
#define PCAL6416A_REG_INPUT0        0x00
#define PCAL6416A_REG_INPUT1        0x01
#define PCAL6416A_REG_OUTPUT0       0x02
#define PCAL6416A_REG_OUTPUT1       0x03
#define PCAL6416A_REG_POLARITY0     0x04
#define PCAL6416A_REG_POLARITY1     0x05
#define PCAL6416A_REG_CONFIG0       0x06
#define PCAL6416A_REG_CONFIG1       0x07
#define PCAL6416A_REG_DRIVE0_0      0x40
#define PCAL6416A_REG_DRIVE0_1      0x41
#define PCAL6416A_REG_DRIVE1_0      0x42
#define PCAL6416A_REG_DRIVE1_1      0x43
#define PCAL6416A_REG_LATCH0        0x44
#define PCAL6416A_REG_LATCH1        0x45
#define PCAL6416A_REG_PULLUP0       0x46
#define PCAL6416A_REG_PULLUP1       0x47
#define PCAL6416A_REG_PULLDOWN0     0x48
#define PCAL6416A_REG_PULLDOWN1     0x49
#define PCAL6416A_REG_INTMASK0      0x4A
#define PCAL6416A_REG_INTMASK1      0x4B
#define PCAL6416A_REG_INTSTAT0      0x4C
#define PCAL6416A_REG_INTSTAT1      0x4D
#define PCAL6416A_REG_OUTCONF       0x4F

// PCAL6416A Device Driver (16-bit GPIO expander with advanced features)
class PCAL6416ADriver {
public:
    PCAL6416ADriver();
    
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
    
#if POCKETOS_PCAL6416A_ENABLE_CONFIGURATION
    // Tier 1: Advanced features
    bool setPullUp(uint8_t pin, bool enable);
    bool setPullDown(uint8_t pin, bool enable);
    bool setPolarity(uint8_t pin, bool inverted);
    bool setDriveStrength(uint8_t pin, uint8_t strength);
    bool enableInterrupt(uint8_t pin);
    bool disableInterrupt(uint8_t pin);
    uint16_t getInterruptStatus();
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pcal6416a"; }
    String getDriverTier() const { return POCKETOS_PCAL6416A_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = PCAL6416A_ADDR_COUNT;
        return PCAL6416A_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < PCAL6416A_ADDR_COUNT; i++) {
            if (PCAL6416A_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_PCAL6416A_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_PCAL6416A_ENABLE_LOGGING
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

#endif // POCKETOS_PCAL6416A_DRIVER_H
