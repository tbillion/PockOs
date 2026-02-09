#ifndef POCKETOS_BMP280_DRIVER_H
#define POCKETOS_BMP280_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_BMP280_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// BMP280 valid I2C addresses
#define BMP280_ADDR_COUNT 2
const uint8_t BMP280_VALID_ADDRESSES[BMP280_ADDR_COUNT] = { 0x76, 0x77 };

// BMP280 calibration data structure
struct BMP280CalibrationData {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    int32_t  t_fine;
};

// BMP280 measurement data
struct BMP280Data {
    float temperature;  // Celsius
    float pressure;     // hPa
    bool valid;
    
    BMP280Data() : temperature(0), pressure(0), valid(false) {}
};

// BMP280 Device Driver
class BMP280Driver {
public:
    BMP280Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    BMP280Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "bmp280"; }
    String getDriverTier() const { return POCKETOS_BMP280_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = BMP280_ADDR_COUNT;
        return BMP280_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < BMP280_ADDR_COUNT; i++) {
            if (BMP280_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_BMP280_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    BMP280CalibrationData calibration;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Calibration
    bool readCalibrationData();
    
    // Compensation formulas
    int32_t compensateTemperature(int32_t adc_T);
    uint32_t compensatePressure(int32_t adc_P);
};

} // namespace PocketOS

#endif // POCKETOS_BMP280_DRIVER_H
