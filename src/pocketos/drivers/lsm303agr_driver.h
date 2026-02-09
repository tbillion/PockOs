#ifndef POCKETOS_LSM303AGR_DRIVER_H
#define POCKETOS_LSM303AGR_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_LSM303AGR_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// LSM303AGR valid I2C addresses (accel at 0x19, mag at 0x1E)
#define LSM303AGR_ADDR_COUNT 2
const uint8_t LSM303AGR_VALID_ADDRESSES[LSM303AGR_ADDR_COUNT] = { 0x19, 0x1E };

// LSM303AGR measurement data
struct LSM303AGRData {
    float accel_x, accel_y, accel_z;  // m/s²
    float mag_x, mag_y, mag_z;        // µT
    float temperature;                 // °C
    bool valid;
    
    LSM303AGRData() : accel_x(0), accel_y(0), accel_z(0),
                      mag_x(0), mag_y(0), mag_z(0),
                      temperature(0), valid(false) {}
};

// LSM303AGR Dual-chip Accelerometer + Magnetometer
class LSM303AGRDriver {
public:
    LSM303AGRDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    LSM303AGRData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return accelAddr; }
    String getDriverId() const { return "lsm303agr"; }
    String getDriverTier() const { return POCKETOS_LSM303AGR_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = LSM303AGR_ADDR_COUNT;
        return LSM303AGR_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < LSM303AGR_ADDR_COUNT; i++) {
            if (LSM303AGR_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_LSM303AGR_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_LSM303AGR_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setAccelDataRate(uint8_t rate);
    bool setAccelScale(uint8_t scale);
    bool setMagDataRate(uint8_t rate);
#endif
    
private:
    uint8_t accelAddr;
    uint8_t magAddr;
    bool initialized;
    float accelScale;
    float magScale;
    
    // I2C communication
    bool writeRegister(uint8_t addr, uint8_t reg, uint8_t value);
    bool readRegister(uint8_t addr, uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t addr, uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_LSM303AGR_DRIVER_H
