#ifndef POCKETOS_BME280_DRIVER_H
#define POCKETOS_BME280_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// BME280 calibration data structure
struct BME280CalibrationData {
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
    
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
    
    int32_t  t_fine;  // Temperature fine value for compensation
};

// BME280 measurement data
struct BME280Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    float pressure;     // hPa
    bool valid;
    
    BME280Data() : temperature(0), humidity(0), pressure(0), valid(false) {}
};

// BME280 Device Driver
class BME280Driver {
public:
    BME280Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    BME280Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "bme280"; }
    String getDriverTier() const { return POCKETOS_BME280_TIER_NAME; }
    
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    // Advanced diagnostics (FULL tier only)
    String getDiagnostics();
    uint32_t getLastReadTime() const { return lastReadTime; }
    uint32_t getReadCount() const { return readCount; }
#endif
    
private:
    uint8_t address;
    bool initialized;
    BME280CalibrationData calibration;
    
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    uint32_t lastReadTime;
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Calibration
    bool readCalibrationData();
    
    // Compensation formulas
    int32_t compensateTemperature(int32_t adc_T);
    uint32_t compensatePressure(int32_t adc_P);
    uint32_t compensateHumidity(int32_t adc_H);
};

} // namespace PocketOS

#endif // POCKETOS_BME280_DRIVER_H
