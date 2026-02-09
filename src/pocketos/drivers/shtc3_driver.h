#ifndef POCKETOS_SHTC3_DRIVER_H
#define POCKETOS_SHTC3_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// SHTC3 valid I2C addresses
#define SHTC3_ADDR_COUNT 1
const uint8_t SHTC3_VALID_ADDRESSES[SHTC3_ADDR_COUNT] = { 0x70 };

// SHTC3 measurement data
struct SHTC3Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    SHTC3Data() : temperature(0), humidity(0), valid(false) {}
};

// SHTC3 Device Driver (Tier 0/1)
class SHTC3Driver {
public:
    SHTC3Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    SHTC3Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "shtc3"; }
    String getDriverTier() const { return POCKETOS_SHTC3_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = SHTC3_ADDR_COUNT;
        return SHTC3_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SHTC3_ADDR_COUNT; i++) {
            if (SHTC3_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_SHTC3_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool sendCommand(uint16_t cmd);
    bool readData(uint8_t* buffer, size_t len);
    
    // CRC calculation
    uint8_t calculateCRC(const uint8_t* data, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_SHTC3_DRIVER_H
