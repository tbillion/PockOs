#ifndef POCKETOS_SHT35_DRIVER_H
#define POCKETOS_SHT35_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// SHT35 valid I2C addresses
#define SHT35_ADDR_COUNT 2
const uint8_t SHT35_VALID_ADDRESSES[SHT35_ADDR_COUNT] = { 0x44, 0x45 };

// SHT35 measurement data
struct SHT35Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    SHT35Data() : temperature(0), humidity(0), valid(false) {}
};

// SHT35 Device Driver (Tier 0/1)
class SHT35Driver {
public:
    SHT35Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    SHT35Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "sht35"; }
    String getDriverTier() const { return POCKETOS_SHT35_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = SHT35_ADDR_COUNT;
        return SHT35_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SHT35_ADDR_COUNT; i++) {
            if (SHT35_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_SHT35_ENABLE_HEATER
    // Heater control (Tier 1 only)
    bool setHeater(bool enabled);
    bool getHeaterStatus();
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_SHT35_ENABLE_LOGGING
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

#endif // POCKETOS_SHT35_DRIVER_H
