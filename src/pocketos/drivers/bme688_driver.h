#ifndef POCKETOS_BME688_DRIVER_H
#define POCKETOS_BME688_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_BME688_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// BME688 valid I2C addresses
#define BME688_ADDR_COUNT 2
const uint8_t BME688_VALID_ADDRESSES[BME688_ADDR_COUNT] = { 0x76, 0x77 };

// BME688 measurement data
struct BME688Data {
    float temperature;  // Celsius
    float humidity;     // %RH
    float pressure;     // hPa
    float gas;          // kOhms
    bool valid;
    
    BME688Data() : temperature(0), humidity(0), pressure(0), gas(0), valid(false) {}
};

// BME688 Device Driver (Environmental Multi-Sensor)
class BME688Driver {
public:
    BME688Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    BME688Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "bme688"; }
    String getDriverTier() const { return POCKETOS_BME688_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = BME688_ADDR_COUNT;
        return BME688_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < BME688_ADDR_COUNT; i++) {
            if (BME688_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_BME688_ENABLE_REGISTER_ACCESS
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
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Calibration
    bool readCalibrationData();
};

} // namespace PocketOS

#endif // POCKETOS_BME688_DRIVER_H
