#ifndef POCKETOS_PCF8575_DRIVER_H
#define POCKETOS_PCF8575_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_PCF8575_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// PCF8575 valid I2C addresses (A2, A1, A0 pins: 0x20-0x27)
#define PCF8575_ADDR_COUNT 8
const uint8_t PCF8575_VALID_ADDRESSES[PCF8575_ADDR_COUNT] = { 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 
};

// PCF8575 Device Driver (16-bit quasi-bidirectional I/O)
class PCF8575Driver {
public:
    PCF8575Driver();
    
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
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pcf8575"; }
    String getDriverTier() const { return POCKETOS_PCF8575_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = PCF8575_ADDR_COUNT;
        return PCF8575_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < PCF8575_ADDR_COUNT; i++) {
            if (PCF8575_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_PCF8575_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    uint16_t outputState;
    
#if POCKETOS_PCF8575_ENABLE_LOGGING
    uint32_t operationCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool writeWord(uint16_t value);
    bool readWord(uint16_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_PCF8575_DRIVER_H
