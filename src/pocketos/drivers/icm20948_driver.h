#ifndef POCKETOS_ICM20948_DRIVER_H
#define POCKETOS_ICM20948_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_ICM20948_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// ICM20948 valid I2C addresses
#define ICM20948_ADDR_COUNT 2
const uint8_t ICM20948_VALID_ADDRESSES[ICM20948_ADDR_COUNT] = { 0x68, 0x69 };

// ICM20948 measurement data
struct ICM20948Data {
    float accel_x, accel_y, accel_z;   // m/s²
    float gyro_x, gyro_y, gyro_z;       // rad/s
    float mag_x, mag_y, mag_z;          // µT
    float temperature;                   // °C
    bool valid;
    
    ICM20948Data() : accel_x(0), accel_y(0), accel_z(0),
                     gyro_x(0), gyro_y(0), gyro_z(0),
                     mag_x(0), mag_y(0), mag_z(0),
                     temperature(0), valid(false) {}
};

// ICM20948 9-DoF IMU
class ICM20948Driver {
public:
    ICM20948Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    ICM20948Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "icm20948"; }
    String getDriverTier() const { return POCKETOS_ICM20948_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = ICM20948_ADDR_COUNT;
        return ICM20948_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < ICM20948_ADDR_COUNT; i++) {
            if (ICM20948_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_ICM20948_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_ICM20948_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setAccelRange(uint8_t range);
    bool setGyroRange(uint8_t range);
#endif
    
private:
    uint8_t address;
    bool initialized;
    float accelScale;
    float gyroScale;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    bool selectBank(uint8_t bank);
    
    // Magnetometer access (AK09916)
    bool readMagnetometer(float& x, float& y, float& z);
};

} // namespace PocketOS

#endif // POCKETOS_ICM20948_DRIVER_H
