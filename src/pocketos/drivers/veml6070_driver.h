#ifndef POCKETOS_VEML6070_DRIVER_H
#define POCKETOS_VEML6070_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_VEML6070_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// VEML6070 valid I2C addresses (data read from 0x38 LSB, 0x39 MSB)
#define VEML6070_ADDR_COUNT 2
const uint8_t VEML6070_VALID_ADDRESSES[VEML6070_ADDR_COUNT] = { 0x38, 0x39 };

// VEML6070 measurement data
struct VEML6070Data {
    uint16_t uv;
    float uvIndex;
    bool valid;
    
    VEML6070Data() : uv(0), uvIndex(0), valid(false) {}
};

// VEML6070 Device Driver
class VEML6070Driver {
public:
    VEML6070Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    VEML6070Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "veml6070"; }
    String getDriverTier() const { return POCKETOS_VEML6070_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = VEML6070_ADDR_COUNT;
        return VEML6070_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < VEML6070_ADDR_COUNT; i++) {
            if (VEML6070_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_VEML6070_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    uint8_t integrationTime;
    
    // I2C communication
    bool writeCommand(uint8_t cmd);
    bool readData(uint16_t* value);
    
    // UV index calculation
    float calculateUVIndex(uint16_t uv);
};

} // namespace PocketOS

#endif // POCKETOS_VEML6070_DRIVER_H
