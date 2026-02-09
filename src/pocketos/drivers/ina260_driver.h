#ifndef POCKETOS_INA260_DRIVER_H
#define POCKETOS_INA260_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_INA260_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// INA260 valid I2C addresses (0x40-0x4F)
#define INA260_ADDR_COUNT 16
const uint8_t INA260_VALID_ADDRESSES[INA260_ADDR_COUNT] = {
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F
};

// INA260 measurement data
struct INA260Data {
    float busVoltage;      // V
    float current;         // mA
    float power;           // mW
    bool valid;
    
    INA260Data() : busVoltage(0), current(0), power(0), valid(false) {}
};

// INA260 Device Driver (integrated 2mΩ shunt resistor)
class INA260Driver {
public:
    INA260Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    INA260Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ina260"; }
    String getDriverTier() const { return POCKETOS_INA260_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = INA260_ADDR_COUNT;
        return INA260_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < INA260_ADDR_COUNT; i++) {
            if (INA260_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_INA260_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setAveraging(uint8_t samples);
    bool setConversionTime(uint16_t microseconds);
    bool setContinuousMode(bool continuous);
#endif
    
#if POCKETOS_INA260_ENABLE_ALERTS
    // Tier 1: Alert configuration
    bool setAlertLimit(float limitValue);
    bool enableAlert(bool enable);
    bool isAlertActive();
#endif
    
#if POCKETOS_INA260_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_INA260_DRIVER_H
