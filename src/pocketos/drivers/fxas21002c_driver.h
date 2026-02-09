#ifndef POCKETOS_FXAS21002C_DRIVER_H
#define POCKETOS_FXAS21002C_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_FXAS21002C_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// FXAS21002C valid I2C addresses
#define FXAS21002C_ADDR_COUNT 2
const uint8_t FXAS21002C_VALID_ADDRESSES[FXAS21002C_ADDR_COUNT] = { 0x20, 0x21 };

// FXAS21002C measurement data
struct FXAS21002CData {
    float gyro_x, gyro_y, gyro_z;  // rad/s
    float temperature;              // °C
    bool valid;
    
    FXAS21002CData() : gyro_x(0), gyro_y(0), gyro_z(0),
                       temperature(0), valid(false) {}
};

// FXAS21002C Gyroscope
class FXAS21002CDriver {
public:
    FXAS21002CDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    FXAS21002CData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "fxas21002c"; }
    String getDriverTier() const { return POCKETOS_FXAS21002C_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = FXAS21002C_ADDR_COUNT;
        return FXAS21002C_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < FXAS21002C_ADDR_COUNT; i++) {
            if (FXAS21002C_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_FXAS21002C_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_FXAS21002C_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setGyroRange(uint8_t range);
    bool setOutputDataRate(uint8_t odr);
#endif
    
private:
    uint8_t address;
    bool initialized;
    float gyroScale;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_FXAS21002C_DRIVER_H
