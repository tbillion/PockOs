#ifndef POCKETOS_VEML6075_DRIVER_H
#define POCKETOS_VEML6075_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_VEML6075_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// VEML6075 valid I2C addresses
#define VEML6075_ADDR_COUNT 1
const uint8_t VEML6075_VALID_ADDRESSES[VEML6075_ADDR_COUNT] = { 0x10 };

// VEML6075 measurement data
struct VEML6075Data {
    float uva;
    float uvb;
    float uvIndex;
    bool valid;
    
    VEML6075Data() : uva(0), uvb(0), uvIndex(0), valid(false) {}
};

// VEML6075 Device Driver
class VEML6075Driver {
public:
    VEML6075Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    VEML6075Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "veml6075"; }
    String getDriverTier() const { return POCKETOS_VEML6075_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = VEML6075_ADDR_COUNT;
        return VEML6075_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < VEML6075_ADDR_COUNT; i++) {
            if (VEML6075_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_VEML6075_ENABLE_REGISTER_ACCESS
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
    
    // UV calculations
    void calculateUV(uint16_t uva_raw, uint16_t uvb_raw, uint16_t comp1, uint16_t comp2);
};

} // namespace PocketOS

#endif // POCKETOS_VEML6075_DRIVER_H
