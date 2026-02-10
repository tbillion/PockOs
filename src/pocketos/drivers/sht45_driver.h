#ifndef POCKETOS_SHT45_DRIVER_H
#define POCKETOS_SHT45_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// SHT45 valid I2C addresses
#define SHT45_ADDR_COUNT 1
const uint8_t SHT45_VALID_ADDRESSES[SHT45_ADDR_COUNT] = { 0x44 };

// SHT45 measurement data
struct SHT45Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    SHT45Data() : temperature(0), humidity(0), valid(false) {}
};

// SHT45 Device Driver (Tier 0/1)
class SHT45Driver {
public:
    SHT45Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    SHT45Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "sht45"; }
    String getDriverTier() const { return POCKETOS_SHT45_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = SHT45_ADDR_COUNT;
        return SHT45_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SHT45_ADDR_COUNT; i++) {
            if (SHT45_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_SHT45_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool sendCommand(uint8_t cmd);
    bool readData(uint8_t* buffer, size_t len);
    
    // CRC calculation
    uint8_t calculateCRC(const uint8_t* data, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_SHT45_DRIVER_H
