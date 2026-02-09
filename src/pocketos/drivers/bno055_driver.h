#ifndef POCKETOS_BNO055_DRIVER_H
#define POCKETOS_BNO055_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_BNO055_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// BNO055 valid I2C addresses
#define BNO055_ADDR_COUNT 2
const uint8_t BNO055_VALID_ADDRESSES[BNO055_ADDR_COUNT] = { 0x28, 0x29 };

// BNO055 measurement data
struct BNO055Data {
    float accel_x, accel_y, accel_z;      // m/s² 
    float gyro_x, gyro_y, gyro_z;          // rad/s
    float mag_x, mag_y, mag_z;             // µT
    float euler_heading, euler_roll, euler_pitch;  // degrees
    float quat_w, quat_x, quat_y, quat_z;  // Quaternion
    float temperature;                      // °C
    bool valid;
    
    BNO055Data() : accel_x(0), accel_y(0), accel_z(0), 
                   gyro_x(0), gyro_y(0), gyro_z(0),
                   mag_x(0), mag_y(0), mag_z(0),
                   euler_heading(0), euler_roll(0), euler_pitch(0),
                   quat_w(1), quat_x(0), quat_y(0), quat_z(0),
                   temperature(0), valid(false) {}
};

// BNO055 9-DoF IMU with sensor fusion
class BNO055Driver {
public:
    BNO055Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    BNO055Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "bno055"; }
    String getDriverTier() const { return POCKETOS_BNO055_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = BNO055_ADDR_COUNT;
        return BNO055_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < BNO055_ADDR_COUNT; i++) {
            if (BNO055_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_BNO055_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_BNO055_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setOperationMode(uint8_t mode);
    uint8_t getOperationMode();
    bool calibrate();
    uint8_t getCalibrationStatus();
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_BNO055_DRIVER_H
