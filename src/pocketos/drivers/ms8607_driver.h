#ifndef POCKETOS_MS8607_DRIVER_H
#define POCKETOS_MS8607_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MS8607_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// MS8607 valid I2C addresses (pressure sensor and humidity sensor)
#define MS8607_ADDR_COUNT 2
const uint8_t MS8607_VALID_ADDRESSES[MS8607_ADDR_COUNT] = { 0x40, 0x76 };

// MS8607 measurement data
struct MS8607Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    float pressure;     // hPa
    bool valid;
    
    MS8607Data() : temperature(0), humidity(0), pressure(0), valid(false) {}
};

// MS8607 Device Driver (Environmental Multi-Sensor: temp/humidity/pressure)
class MS8607Driver {
public:
    MS8607Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    MS8607Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ms8607"; }
    String getDriverTier() const { return POCKETOS_MS8607_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = MS8607_ADDR_COUNT;
        return MS8607_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MS8607_ADDR_COUNT; i++) {
            if (MS8607_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MS8607_ENABLE_REGISTER_ACCESS
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
    bool sendCommand(uint8_t addr, uint8_t cmd);
    bool readData(uint8_t addr, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_MS8607_DRIVER_H
