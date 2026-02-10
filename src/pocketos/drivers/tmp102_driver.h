#ifndef POCKETOS_TMP102_DRIVER_H
#define POCKETOS_TMP102_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// TMP102 valid I2C addresses
#define TMP102_ADDR_COUNT 4
const uint8_t TMP102_VALID_ADDRESSES[TMP102_ADDR_COUNT] = { 0x48, 0x49, 0x4A, 0x4B };

// TMP102 measurement data
struct TMP102Data {
    float temperature;  // Celsius
    bool valid;
    
    TMP102Data() : temperature(0), valid(false) {}
};

// TMP102 Device Driver (Tier 0/1)
class TMP102Driver {
public:
    TMP102Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    TMP102Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "tmp102"; }
    String getDriverTier() const { return POCKETOS_TMP102_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = TMP102_ADDR_COUNT;
        return TMP102_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < TMP102_ADDR_COUNT; i++) {
            if (TMP102_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_TMP102_ENABLE_CONFIGURATION
    // Configuration (Tier 1 only)
    bool setConversionRate(uint8_t rate);
    bool setExtendedMode(bool enable);
#endif
    
private:
    uint8_t address;
    bool initialized;
    bool extendedMode;
    
#if POCKETOS_TMP102_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool readRegister(uint8_t reg, uint8_t* buffer, size_t len);
    bool writeRegister(uint8_t reg, uint16_t value);
};

} // namespace PocketOS

#endif // POCKETOS_TMP102_DRIVER_H
