#ifndef POCKETOS_AM2315_DRIVER_H
#define POCKETOS_AM2315_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// AM2315 valid I2C addresses
#define AM2315_ADDR_COUNT 1
const uint8_t AM2315_VALID_ADDRESSES[AM2315_ADDR_COUNT] = { 0x5C };

// AM2315 measurement data
struct AM2315Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    AM2315Data() : temperature(0), humidity(0), valid(false) {}
};

// AM2315 Device Driver (Tier 0/1)
class AM2315Driver {
public:
    AM2315Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    AM2315Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "am2315"; }
    String getDriverTier() const { return POCKETOS_AM2315_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = AM2315_ADDR_COUNT;
        return AM2315_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < AM2315_ADDR_COUNT; i++) {
            if (AM2315_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_AM2315_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool wakeup();
    bool readRegisters(uint8_t reg, uint8_t count, uint8_t* buffer);
    
    // CRC calculation
    uint16_t calculateCRC(const uint8_t* data, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_AM2315_DRIVER_H
