#ifndef POCKETOS_LSM6DSOX_DRIVER_H
#define POCKETOS_LSM6DSOX_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_LSM6DSOX_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// LSM6DSOX valid I2C addresses
#define LSM6DSOX_ADDR_COUNT 2
const uint8_t LSM6DSOX_VALID_ADDRESSES[LSM6DSOX_ADDR_COUNT] = { 0x6A, 0x6B };

// LSM6DSOX measurement data
struct LSM6DSOXData {
    float accel_x, accel_y, accel_z;  // m/s²
    float gyro_x, gyro_y, gyro_z;      // rad/s
    float temperature;                  // °C
    bool valid;
    
    LSM6DSOXData() : accel_x(0), accel_y(0), accel_z(0),
                     gyro_x(0), gyro_y(0), gyro_z(0),
                     temperature(0), valid(false) {}
};

// LSM6DSOX 6-axis IMU with ML core (Accelerometer + Gyroscope)
class LSM6DSOXDriver {
public:
    LSM6DSOXDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    LSM6DSOXData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "lsm6dsox"; }
    String getDriverTier() const { return POCKETOS_LSM6DSOX_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = LSM6DSOX_ADDR_COUNT;
        return LSM6DSOX_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < LSM6DSOX_ADDR_COUNT; i++) {
            if (LSM6DSOX_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_LSM6DSOX_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_LSM6DSOX_ENABLE_CONFIGURATION
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
};

} // namespace PocketOS

#endif // POCKETOS_LSM6DSOX_DRIVER_H
