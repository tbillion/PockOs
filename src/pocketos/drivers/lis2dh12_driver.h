#ifndef POCKETOS_LIS2DH12_DRIVER_H
#define POCKETOS_LIS2DH12_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_LIS2DH12_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// LIS2DH12 valid I2C addresses (also works for LIS3DH)
#define LIS2DH12_ADDR_COUNT 2
const uint8_t LIS2DH12_VALID_ADDRESSES[LIS2DH12_ADDR_COUNT] = { 0x18, 0x19 };

// LIS2DH12 measurement data
struct LIS2DH12Data {
    float accel_x, accel_y, accel_z;  // m/s²
    float temperature;                 // °C
    bool valid;
    
    LIS2DH12Data() : accel_x(0), accel_y(0), accel_z(0), temperature(0), valid(false) {}
};

// LIS2DH12 3-axis Accelerometer (also works for LIS3DH)
class LIS2DH12Driver {
public:
    LIS2DH12Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    LIS2DH12Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "lis2dh12"; }
    String getDriverTier() const { return POCKETOS_LIS2DH12_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = LIS2DH12_ADDR_COUNT;
        return LIS2DH12_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < LIS2DH12_ADDR_COUNT; i++) {
            if (LIS2DH12_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_LIS2DH12_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_LIS2DH12_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setDataRate(uint8_t rate);
    bool setScale(uint8_t scale);
#endif
    
private:
    uint8_t address;
    bool initialized;
    float accelScale;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_LIS2DH12_DRIVER_H
