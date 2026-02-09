#ifndef POCKETOS_MS5611_DRIVER_H
#define POCKETOS_MS5611_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MS5611_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// MS5611 valid I2C addresses
#define MS5611_ADDR_COUNT 1
const uint8_t MS5611_VALID_ADDRESSES[MS5611_ADDR_COUNT] = { 0x77 };

// MS5611 calibration data structure
struct MS5611CalibrationData {
    uint16_t c1;  // Pressure sensitivity
    uint16_t c2;  // Pressure offset
    uint16_t c3;  // Temperature coefficient of pressure sensitivity
    uint16_t c4;  // Temperature coefficient of pressure offset
    uint16_t c5;  // Reference temperature
    uint16_t c6;  // Temperature coefficient of the temperature
};

// MS5611 measurement data
struct MS5611Data {
    float temperature;  // Celsius
    float pressure;     // hPa
    bool valid;
    
    MS5611Data() : temperature(0), pressure(0), valid(false) {}
};

// MS5611 Device Driver
class MS5611Driver {
public:
    MS5611Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    MS5611Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ms5611"; }
    String getDriverTier() const { return POCKETOS_MS5611_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = MS5611_ADDR_COUNT;
        return MS5611_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MS5611_ADDR_COUNT; i++) {
            if (MS5611_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MS5611_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    MS5611CalibrationData calibration;
    
    // I2C communication
    bool sendCommand(uint8_t cmd);
    bool readADC(uint32_t* value);
    
    // Calibration
    bool readCalibrationData();
    
    // Raw readings
    uint32_t readRawTemperature();
    uint32_t readRawPressure();
};

} // namespace PocketOS

#endif // POCKETOS_MS5611_DRIVER_H
