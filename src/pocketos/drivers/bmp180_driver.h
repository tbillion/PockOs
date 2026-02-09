#ifndef POCKETOS_BMP180_DRIVER_H
#define POCKETOS_BMP180_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_BMP180_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// BMP180 valid I2C addresses
#define BMP180_ADDR_COUNT 1
const uint8_t BMP180_VALID_ADDRESSES[BMP180_ADDR_COUNT] = { 0x77 };

// BMP180 calibration data structure
struct BMP180CalibrationData {
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
};

// BMP180 measurement data
struct BMP180Data {
    float temperature;  // Celsius
    float pressure;     // hPa
    bool valid;
    
    BMP180Data() : temperature(0), pressure(0), valid(false) {}
};

// BMP180 Device Driver
class BMP180Driver {
public:
    BMP180Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    BMP180Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "bmp180"; }
    String getDriverTier() const { return POCKETOS_BMP180_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = BMP180_ADDR_COUNT;
        return BMP180_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < BMP180_ADDR_COUNT; i++) {
            if (BMP180_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_BMP180_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    BMP180CalibrationData calibration;
    uint8_t oversampling;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Calibration
    bool readCalibrationData();
    
    // Raw readings
    int32_t readRawTemperature();
    int32_t readRawPressure();
};

} // namespace PocketOS

#endif // POCKETOS_BMP180_DRIVER_H
