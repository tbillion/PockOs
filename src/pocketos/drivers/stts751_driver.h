#ifndef POCKETOS_STTS751_DRIVER_H
#define POCKETOS_STTS751_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// STTS751 valid I2C addresses
#define STTS751_ADDR_COUNT 4
const uint8_t STTS751_VALID_ADDRESSES[STTS751_ADDR_COUNT] = { 0x39, 0x3A, 0x3B, 0x3C };

// STTS751 measurement data
struct STTS751Data {
    float temperature;  // Celsius
    bool valid;
    
    STTS751Data() : temperature(0), valid(false) {}
};

// STTS751 Device Driver (Tier 0/1)
class STTS751Driver {
public:
    STTS751Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    STTS751Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "stts751"; }
    String getDriverTier() const { return POCKETOS_STTS751_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = STTS751_ADDR_COUNT;
        return STTS751_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < STTS751_ADDR_COUNT; i++) {
            if (STTS751_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_STTS751_ENABLE_CONFIGURATION
    // Configuration (Tier 1 only)
    bool setResolution(uint8_t resolution);
    bool setConversionRate(uint8_t rate);
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_STTS751_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool readRegister(uint8_t reg, uint8_t* value);
    bool writeRegister(uint8_t reg, uint8_t value);
};

} // namespace PocketOS

#endif // POCKETOS_STTS751_DRIVER_H
