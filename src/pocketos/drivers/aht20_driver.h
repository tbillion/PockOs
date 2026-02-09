#ifndef POCKETOS_AHT20_DRIVER_H
#define POCKETOS_AHT20_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// AHT20 valid I2C addresses
#define AHT20_ADDR_COUNT 1
const uint8_t AHT20_VALID_ADDRESSES[AHT20_ADDR_COUNT] = { 0x38 };

// AHT20 measurement data
struct AHT20Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    AHT20Data() : temperature(0), humidity(0), valid(false) {}
};

// AHT20 Device Driver (Tier 0/1)
class AHT20Driver {
public:
    AHT20Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    AHT20Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "aht20"; }
    String getDriverTier() const { return POCKETOS_AHT20_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = AHT20_ADDR_COUNT;
        return AHT20_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < AHT20_ADDR_COUNT; i++) {
            if (AHT20_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_AHT20_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool sendCommand(uint8_t cmd, uint8_t param1, uint8_t param2);
    bool readData(uint8_t* buffer, size_t len);
    
    // CRC calculation
    uint8_t calculateCRC(const uint8_t* data, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_AHT20_DRIVER_H
