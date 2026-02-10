#ifndef POCKETOS_INA226_DRIVER_H
#define POCKETOS_INA226_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_INA226_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// INA226 valid I2C addresses (0x40-0x4F)
#define INA226_ADDR_COUNT 16
const uint8_t INA226_VALID_ADDRESSES[INA226_ADDR_COUNT] = {
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F
};

// INA226 measurement data
struct INA226Data {
    float busVoltage;      // V
    float shuntVoltage;    // mV
    float current;         // mA
    float power;           // mW
    bool valid;
    
    INA226Data() : busVoltage(0), shuntVoltage(0), current(0), power(0), valid(false) {}
};

// INA226 Device Driver
class INA226Driver {
public:
    INA226Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    INA226Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ina226"; }
    String getDriverTier() const { return POCKETOS_INA226_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = INA226_ADDR_COUNT;
        return INA226_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < INA226_ADDR_COUNT; i++) {
            if (INA226_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_INA226_ENABLE_CALIBRATION
    // Tier 1: Calibration and configuration
    bool setCalibration(float shuntResistorOhms, float maxCurrentA);
    bool setAveraging(uint8_t samples);
    bool setConversionTime(uint16_t microseconds);
#endif
    
#if POCKETOS_INA226_ENABLE_ALERTS
    // Tier 1: Alert configuration
    bool setAlertLimit(float limitValue);
    bool enableAlert(bool enable);
    bool isAlertActive();
#endif
    
#if POCKETOS_INA226_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_INA226_DRIVER_H
