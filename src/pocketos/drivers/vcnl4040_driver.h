#ifndef POCKETOS_VCNL4040_DRIVER_H
#define POCKETOS_VCNL4040_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_VCNL4040_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// VCNL4040 valid I2C addresses
#define VCNL4040_ADDR_COUNT 1
const uint8_t VCNL4040_VALID_ADDRESSES[VCNL4040_ADDR_COUNT] = { 0x60 };

// VCNL4040 measurement data
struct VCNL4040Data {
    uint16_t proximity;
    float ambient;
    uint16_t white;
    bool valid;
    
    VCNL4040Data() : proximity(0), ambient(0), white(0), valid(false) {}
};

// VCNL4040 Device Driver
class VCNL4040Driver {
public:
    VCNL4040Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    VCNL4040Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "vcnl4040"; }
    String getDriverTier() const { return POCKETOS_VCNL4040_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = VCNL4040_ADDR_COUNT;
        return VCNL4040_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < VCNL4040_ADDR_COUNT; i++) {
            if (VCNL4040_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_VCNL4040_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_VCNL4040_DRIVER_H
