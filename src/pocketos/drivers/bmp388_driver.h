#ifndef POCKETOS_BMP388_DRIVER_H
#define POCKETOS_BMP388_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_BMP388_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// BMP388 valid I2C addresses
#define BMP388_ADDR_COUNT 2
const uint8_t BMP388_VALID_ADDRESSES[BMP388_ADDR_COUNT] = { 0x76, 0x77 };

// BMP388 calibration data structure
struct BMP388CalibrationData {
    uint16_t par_t1;
    uint16_t par_t2;
    int8_t   par_t3;
    int16_t  par_p1;
    int16_t  par_p2;
    int8_t   par_p3;
    int8_t   par_p4;
    uint16_t par_p5;
    uint16_t par_p6;
    int8_t   par_p7;
    int8_t   par_p8;
    int16_t  par_p9;
    int8_t   par_p10;
    int8_t   par_p11;
    int64_t  t_lin;
};

// BMP388 measurement data
struct BMP388Data {
    float temperature;  // Celsius
    float pressure;     // hPa
    bool valid;
    
    BMP388Data() : temperature(0), pressure(0), valid(false) {}
};

// BMP388 Device Driver
class BMP388Driver {
public:
    BMP388Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    BMP388Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "bmp388"; }
    String getDriverTier() const { return POCKETOS_BMP388_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = BMP388_ADDR_COUNT;
        return BMP388_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < BMP388_ADDR_COUNT; i++) {
            if (BMP388_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_BMP388_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    BMP388CalibrationData calibration;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Calibration
    bool readCalibrationData();
    
    // Compensation formulas
    float compensateTemperature(uint32_t adc_T);
    float compensatePressure(uint32_t adc_P);
};

} // namespace PocketOS

#endif // POCKETOS_BMP388_DRIVER_H
