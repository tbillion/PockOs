#ifndef POCKETOS_AS6212_DRIVER_H
#define POCKETOS_AS6212_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// AS6212 valid I2C addresses
#define AS6212_ADDR_COUNT 4
const uint8_t AS6212_VALID_ADDRESSES[AS6212_ADDR_COUNT] = { 0x48, 0x49, 0x4A, 0x4B };

// AS6212 measurement data
struct AS6212Data {
    float temperature;  // Celsius
    bool valid;
    
    AS6212Data() : temperature(0), valid(false) {}
};

// AS6212 Device Driver (Tier 0/1)
class AS6212Driver {
public:
    AS6212Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    AS6212Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "as6212"; }
    String getDriverTier() const { return POCKETOS_AS6212_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = AS6212_ADDR_COUNT;
        return AS6212_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < AS6212_ADDR_COUNT; i++) {
            if (AS6212_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_AS6212_ENABLE_CONFIGURATION
    // Configuration (Tier 1 only)
    bool setConversionRate(uint8_t rate);
    bool setAlertPolarity(bool activeHigh);
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_AS6212_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool readRegister(uint8_t reg, uint8_t* buffer, size_t len);
    bool writeRegister(uint8_t reg, uint16_t value);
};

} // namespace PocketOS

#endif // POCKETOS_AS6212_DRIVER_H
