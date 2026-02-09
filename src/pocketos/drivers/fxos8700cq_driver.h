#ifndef POCKETOS_FXOS8700CQ_DRIVER_H
#define POCKETOS_FXOS8700CQ_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_FXOS8700CQ_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// FXOS8700CQ valid I2C addresses
#define FXOS8700CQ_ADDR_COUNT 1
const uint8_t FXOS8700CQ_VALID_ADDRESSES[FXOS8700CQ_ADDR_COUNT] = { 0x1E };

// FXOS8700CQ measurement data
struct FXOS8700CQData {
    float accel_x, accel_y, accel_z;  // m/s²
    float mag_x, mag_y, mag_z;        // µT
    float temperature;                 // °C
    bool valid;
    
    FXOS8700CQData() : accel_x(0), accel_y(0), accel_z(0),
                       mag_x(0), mag_y(0), mag_z(0),
                       temperature(0), valid(false) {}
};

// FXOS8700CQ Accelerometer + Magnetometer
class FXOS8700CQDriver {
public:
    FXOS8700CQDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    FXOS8700CQData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "fxos8700cq"; }
    String getDriverTier() const { return POCKETOS_FXOS8700CQ_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = FXOS8700CQ_ADDR_COUNT;
        return FXOS8700CQ_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < FXOS8700CQ_ADDR_COUNT; i++) {
            if (FXOS8700CQ_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_FXOS8700CQ_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_FXOS8700CQ_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setAccelRange(uint8_t range);
    bool setHybridMode(bool enable);
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

#endif // POCKETOS_FXOS8700CQ_DRIVER_H
