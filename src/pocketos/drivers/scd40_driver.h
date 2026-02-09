#ifndef POCKETOS_SCD40_DRIVER_H
#define POCKETOS_SCD40_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_SCD40_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// SCD40 valid I2C addresses
#define SCD40_ADDR_COUNT 1
const uint8_t SCD40_VALID_ADDRESSES[SCD40_ADDR_COUNT] = { 0x62 };

// SCD40 measurement data
struct SCD40Data {
    float co2;          // ppm
    float temperature;  // Celsius
    float humidity;     // %RH
    bool valid;
    
    SCD40Data() : co2(0), temperature(0), humidity(0), valid(false) {}
};

// SCD40 Device Driver (CO2/Temp/Humidity)
class SCD40Driver {
public:
    SCD40Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    SCD40Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "scd40"; }
    String getDriverTier() const { return POCKETOS_SCD40_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = SCD40_ADDR_COUNT;
        return SCD40_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SCD40_ADDR_COUNT; i++) {
            if (SCD40_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_SCD40_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    // I2C communication
    bool sendCommand(uint16_t cmd);
    bool readData(uint8_t* buffer, size_t len);
    uint8_t computeCRC(uint8_t data[], uint8_t len);
};

} // namespace PocketOS

#endif // POCKETOS_SCD40_DRIVER_H
