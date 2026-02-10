#ifndef POCKETOS_TMP117_DRIVER_H
#define POCKETOS_TMP117_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// TMP117 valid I2C addresses
#define TMP117_ADDR_COUNT 4
const uint8_t TMP117_VALID_ADDRESSES[TMP117_ADDR_COUNT] = { 0x48, 0x49, 0x4A, 0x4B };

// TMP117 measurement data
struct TMP117Data {
    float temperature;  // Celsius
    bool valid;
    
    TMP117Data() : temperature(0), valid(false) {}
};

// TMP117 Device Driver (Tier 0/1)
class TMP117Driver {
public:
    TMP117Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    TMP117Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "tmp117"; }
    String getDriverTier() const { return POCKETOS_TMP117_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = TMP117_ADDR_COUNT;
        return TMP117_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < TMP117_ADDR_COUNT; i++) {
            if (TMP117_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_TMP117_ENABLE_CONFIGURATION
    // Configuration (Tier 1 only)
    bool setConversionCycleTime(uint8_t cycleTime);
    bool setAveraging(uint8_t averaging);
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_TMP117_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool readRegister(uint8_t reg, uint8_t* buffer, size_t len);
    bool writeRegister(uint8_t reg, uint16_t value);
};

} // namespace PocketOS

#endif // POCKETOS_TMP117_DRIVER_H
