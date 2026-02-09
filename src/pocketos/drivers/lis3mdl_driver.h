#ifndef POCKETOS_LIS3MDL_DRIVER_H
#define POCKETOS_LIS3MDL_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_LIS3MDL_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// LIS3MDL valid I2C addresses
#define LIS3MDL_ADDR_COUNT 2
const uint8_t LIS3MDL_VALID_ADDRESSES[LIS3MDL_ADDR_COUNT] = { 0x1C, 0x1E };

// LIS3MDL measurement data
struct LIS3MDLData {
    float mag_x, mag_y, mag_z;  // µT
    float temperature;          // °C
    bool valid;
    
    LIS3MDLData() : mag_x(0), mag_y(0), mag_z(0), temperature(0), valid(false) {}
};

// LIS3MDL 3-axis Magnetometer
class LIS3MDLDriver {
public:
    LIS3MDLDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    LIS3MDLData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "lis3mdl"; }
    String getDriverTier() const { return POCKETOS_LIS3MDL_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = LIS3MDL_ADDR_COUNT;
        return LIS3MDL_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < LIS3MDL_ADDR_COUNT; i++) {
            if (LIS3MDL_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_LIS3MDL_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_LIS3MDL_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setDataRate(uint8_t rate);
    bool setScale(uint8_t scale);
#endif
    
private:
    uint8_t address;
    bool initialized;
    float magScale;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_LIS3MDL_DRIVER_H
