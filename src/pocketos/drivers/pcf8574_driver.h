#ifndef POCKETOS_PCF8574_DRIVER_H
#define POCKETOS_PCF8574_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_PCF8574_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// PCF8574 valid I2C addresses (A2, A1, A0 pins: 0x20-0x27, 0x38-0x3F)
#define PCF8574_ADDR_COUNT 16
const uint8_t PCF8574_VALID_ADDRESSES[PCF8574_ADDR_COUNT] = { 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

// PCF8574 Device Driver (8-bit quasi-bidirectional I/O)
class PCF8574Driver {
public:
    PCF8574Driver();
    
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
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pcf8574"; }
    String getDriverTier() const { return POCKETOS_PCF8574_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = PCF8574_ADDR_COUNT;
        return PCF8574_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < PCF8574_ADDR_COUNT; i++) {
            if (PCF8574_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_PCF8574_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    uint8_t outputState;
    
#if POCKETOS_PCF8574_ENABLE_LOGGING
    uint32_t operationCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool writeByte(uint8_t value);
    bool readByte(uint8_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_PCF8574_DRIVER_H
