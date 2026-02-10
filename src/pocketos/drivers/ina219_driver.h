#ifndef POCKETOS_INA219_DRIVER_H
#define POCKETOS_INA219_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_INA219_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// INA219 valid I2C addresses (0x40-0x4F)
#define INA219_ADDR_COUNT 16
const uint8_t INA219_VALID_ADDRESSES[INA219_ADDR_COUNT] = {
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F
};

// INA219 measurement data
struct INA219Data {
    float busVoltage;      // V
    float shuntVoltage;    // mV
    float current;         // mA
    float power;           // mW
    bool valid;
    
    INA219Data() : busVoltage(0), shuntVoltage(0), current(0), power(0), valid(false) {}
};

// INA219 Device Driver
class INA219Driver {
public:
    INA219Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    INA219Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ina219"; }
    String getDriverTier() const { return POCKETOS_INA219_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = INA219_ADDR_COUNT;
        return INA219_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < INA219_ADDR_COUNT; i++) {
            if (INA219_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_INA219_ENABLE_CALIBRATION
    // Tier 1: Calibration
    bool setCalibration(float shuntResistorOhms, float maxCurrentA);
#endif
    
#if POCKETOS_INA219_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    float currentLSB;
    float powerLSB;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_INA219_DRIVER_H
