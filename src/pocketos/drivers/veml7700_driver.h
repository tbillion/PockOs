#ifndef POCKETOS_VEML7700_DRIVER_H
#define POCKETOS_VEML7700_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_VEML7700_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// VEML7700 valid I2C addresses
#define VEML7700_ADDR_COUNT 1
const uint8_t VEML7700_VALID_ADDRESSES[VEML7700_ADDR_COUNT] = { 0x10 };

// VEML7700 measurement data
struct VEML7700Data {
    float lux;
    float white;
    uint16_t als;
    bool valid;
    
    VEML7700Data() : lux(0), white(0), als(0), valid(false) {}
};

// VEML7700 Device Driver
class VEML7700Driver {
public:
    VEML7700Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    VEML7700Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "veml7700"; }
    String getDriverTier() const { return POCKETOS_VEML7700_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = VEML7700_ADDR_COUNT;
        return VEML7700_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < VEML7700_ADDR_COUNT; i++) {
            if (VEML7700_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_VEML7700_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    uint8_t gain;
    uint8_t integrationTime;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t* value);
    
    // Lux calculation
    float calculateLux(uint16_t als);
};

} // namespace PocketOS

#endif // POCKETOS_VEML7700_DRIVER_H
