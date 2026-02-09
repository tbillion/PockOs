#ifndef POCKETOS_SI7021_DRIVER_H
#define POCKETOS_SI7021_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// SI7021 valid I2C addresses
#define SI7021_ADDR_COUNT 1
const uint8_t SI7021_VALID_ADDRESSES[SI7021_ADDR_COUNT] = { 0x40 };

// SI7021 measurement data
struct SI7021Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    SI7021Data() : temperature(0), humidity(0), valid(false) {}
};

// SI7021 Device Driver (Tier 0/1)
class SI7021Driver {
public:
    SI7021Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    SI7021Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "si7021"; }
    String getDriverTier() const { return POCKETOS_SI7021_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = SI7021_ADDR_COUNT;
        return SI7021_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SI7021_ADDR_COUNT; i++) {
            if (SI7021_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_SI7021_ENABLE_HEATER
    // Heater control (Tier 1 only)
    bool setHeater(bool enabled);
    bool getHeaterStatus();
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_SI7021_ENABLE_LOGGING
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

#endif // POCKETOS_SI7021_DRIVER_H
