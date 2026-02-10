#ifndef POCKETOS_HMC5883L_DRIVER_H
#define POCKETOS_HMC5883L_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_HMC5883L_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// HMC5883L valid I2C addresses
#define HMC5883L_ADDR_COUNT 1
const uint8_t HMC5883L_VALID_ADDRESSES[HMC5883L_ADDR_COUNT] = { 0x1E };

// HMC5883L measurement data
struct HMC5883LData {
    float mag_x, mag_y, mag_z;  // µT
    bool valid;
    
    HMC5883LData() : mag_x(0), mag_y(0), mag_z(0), valid(false) {}
};

// HMC5883L 3-axis Magnetometer
class HMC5883LDriver {
public:
    HMC5883LDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    HMC5883LData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "hmc5883l"; }
    String getDriverTier() const { return POCKETOS_HMC5883L_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = HMC5883L_ADDR_COUNT;
        return HMC5883L_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < HMC5883L_ADDR_COUNT; i++) {
            if (HMC5883L_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_HMC5883L_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
#if POCKETOS_HMC5883L_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setGain(uint8_t gain);
    bool setMode(uint8_t mode);
#endif
    
private:
    uint8_t address;
    bool initialized;
    float magGain;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_HMC5883L_DRIVER_H
