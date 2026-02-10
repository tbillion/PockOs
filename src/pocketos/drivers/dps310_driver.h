#ifndef POCKETOS_DPS310_DRIVER_H
#define POCKETOS_DPS310_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_DPS310_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// DPS310 valid I2C addresses
#define DPS310_ADDR_COUNT 1
const uint8_t DPS310_VALID_ADDRESSES[DPS310_ADDR_COUNT] = { 0x77 };

// DPS310 calibration data structure
struct DPS310CalibrationData {
    int32_t c0;
    int32_t c1;
    int32_t c00;
    int32_t c10;
    int32_t c01;
    int32_t c11;
    int32_t c20;
    int32_t c21;
    int32_t c30;
};

// DPS310 measurement data
struct DPS310Data {
    float temperature;  // Celsius
    float pressure;     // hPa
    bool valid;
    
    DPS310Data() : temperature(0), pressure(0), valid(false) {}
};

// DPS310 Device Driver
class DPS310Driver {
public:
    DPS310Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    DPS310Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "dps310"; }
    String getDriverTier() const { return POCKETOS_DPS310_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = DPS310_ADDR_COUNT;
        return DPS310_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < DPS310_ADDR_COUNT; i++) {
            if (DPS310_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_DPS310_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    DPS310CalibrationData calibration;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Calibration
    bool readCalibrationData();
    
    // Compensation formulas
    float compensateTemperature(int32_t raw);
    float compensatePressure(int32_t raw, float temp_scaled);
};

} // namespace PocketOS

#endif // POCKETOS_DPS310_DRIVER_H
