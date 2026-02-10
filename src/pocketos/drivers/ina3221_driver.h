#ifndef POCKETOS_INA3221_DRIVER_H
#define POCKETOS_INA3221_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_INA3221_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// INA3221 valid I2C addresses (0x40-0x43)
#define INA3221_ADDR_COUNT 4
const uint8_t INA3221_VALID_ADDRESSES[INA3221_ADDR_COUNT] = {
    0x40, 0x41, 0x42, 0x43
};

// INA3221 channel data
struct INA3221ChannelData {
    float busVoltage;      // V
    float shuntVoltage;    // mV
    float current;         // mA
    bool valid;
    
    INA3221ChannelData() : busVoltage(0), shuntVoltage(0), current(0), valid(false) {}
};

// INA3221 measurement data (3 channels)
struct INA3221Data {
    INA3221ChannelData channel[3];
    bool valid;
    
    INA3221Data() : valid(false) {}
};

// INA3221 Device Driver (3-channel power monitor)
class INA3221Driver {
public:
    INA3221Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    INA3221Data readData();
    INA3221ChannelData readChannel(uint8_t channel);
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ina3221"; }
    String getDriverTier() const { return POCKETOS_INA3221_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = INA3221_ADDR_COUNT;
        return INA3221_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < INA3221_ADDR_COUNT; i++) {
            if (INA3221_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_INA3221_ENABLE_CONFIGURATION
    // Tier 1: Configuration
    bool setShuntResistor(uint8_t channel, float resistorOhms);
    bool setAveraging(uint8_t samples);
    bool setConversionTime(uint16_t microseconds);
    bool enableChannel(uint8_t channel, bool enable);
#endif
    
#if POCKETOS_INA3221_ENABLE_ALERTS
    // Tier 1: Alert configuration
    bool setWarningLimit(uint8_t channel, float limitV);
    bool setCriticalLimit(uint8_t channel, float limitV);
    bool enableWarningAlert(uint8_t channel, bool enable);
    bool enableCriticalAlert(uint8_t channel, bool enable);
#endif
    
#if POCKETOS_INA3221_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    float shuntResistor[3];  // Ohms for each channel
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_INA3221_DRIVER_H
