#ifndef POCKETOS_SHT40_DRIVER_H
#define POCKETOS_SHT40_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// SHT40 valid I2C addresses
#define SHT40_ADDR_COUNT 1
const uint8_t SHT40_VALID_ADDRESSES[SHT40_ADDR_COUNT] = { 0x44 };

// SHT40 measurement data
struct SHT40Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    SHT40Data() : temperature(0), humidity(0), valid(false) {}
};

// SHT40 Device Driver (Tier 0/1)
class SHT40Driver {
public:
    SHT40Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    SHT40Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "sht40"; }
    String getDriverTier() const { return POCKETOS_SHT40_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = SHT40_ADDR_COUNT;
        return SHT40_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SHT40_ADDR_COUNT; i++) {
            if (SHT40_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_SHT40_ENABLE_LOGGING
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

#endif // POCKETOS_SHT40_DRIVER_H
