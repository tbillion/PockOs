#ifndef POCKETOS_ENS160_DRIVER_H
#define POCKETOS_ENS160_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_ENS160_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// ENS160 valid I2C addresses
#define ENS160_ADDR_COUNT 2
const uint8_t ENS160_VALID_ADDRESSES[ENS160_ADDR_COUNT] = { 0x52, 0x53 };

// ENS160 air quality index
enum class AQI {
    EXCELLENT = 1,
    GOOD = 2,
    MODERATE = 3,
    POOR = 4,
    UNHEALTHY = 5
};

// ENS160 data
struct ENS160Data {
    uint16_t tvoc;      // TVOC in ppb
    uint16_t eco2;      // eCO2 in ppm
    AQI aqi;            // Air quality index
    bool valid;
    
    ENS160Data() : tvoc(0), eco2(0), aqi(AQI::EXCELLENT), valid(false) {}
};

class ENS160Driver {
public:
    ENS160Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    ENS160Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ens160"; }
    String getDriverTier() const { return POCKETOS_ENS160_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = ENS160_ADDR_COUNT;
        return ENS160_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < ENS160_ADDR_COUNT; i++) {
            if (ENS160_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_ENS160_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool readRegister(uint8_t reg, uint8_t* buffer, size_t length);
    bool writeRegister(uint8_t reg, uint8_t value);
};

} // namespace PocketOS

#endif
