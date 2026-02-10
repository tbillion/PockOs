#ifndef POCKETOS_LSM9DS1_DRIVER_H
#define POCKETOS_LSM9DS1_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_LSM9DS1_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// LSM9DS1 valid I2C addresses (AG chip: 0x6A/0x6B, Mag chip: 0x1C/0x1E)
#define LSM9DS1_ADDR_COUNT 4
const uint8_t LSM9DS1_VALID_ADDRESSES[LSM9DS1_ADDR_COUNT] = { 0x6A, 0x6B, 0x1C, 0x1E };

// LSM9DS1 measurement data
struct LSM9DS1Data {
    float accel_x, accel_y, accel_z;  // m/s²
    float gyro_x, gyro_y, gyro_z;      // rad/s
    float mag_x, mag_y, mag_z;         // µT
    float temperature;                  // °C
    bool valid;
    
    LSM9DS1Data() : accel_x(0), accel_y(0), accel_z(0),
                    gyro_x(0), gyro_y(0), gyro_z(0),
                    mag_x(0), mag_y(0), mag_z(0),
                    temperature(0), valid(false) {}
};

// LSM9DS1 9-DoF dual-chip IMU (Accelerometer + Gyroscope + Magnetometer)
class LSM9DS1Driver {
public:
    LSM9DS1Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    LSM9DS1Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return agAddress; }
    String getDriverId() const { return "lsm9ds1"; }
    String getDriverTier() const { return POCKETOS_LSM9DS1_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = LSM9DS1_ADDR_COUNT;
        return LSM9DS1_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < LSM9DS1_ADDR_COUNT; i++) {
            if (LSM9DS1_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_LSM9DS1_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_LSM9DS1_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setAccelRange(uint8_t range);
    bool setGyroRange(uint8_t range);
    bool setMagRange(uint8_t range);
#endif
    
private:
    uint8_t agAddress;
    uint8_t magAddress;
    bool initialized;
    float accelScale;
    float gyroScale;
    float magScale;
    
    // I2C communication
    bool writeRegister(uint8_t addr, uint8_t reg, uint8_t value);
    bool readRegister(uint8_t addr, uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t addr, uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_LSM9DS1_DRIVER_H
