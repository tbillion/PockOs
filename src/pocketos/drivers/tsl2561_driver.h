#ifndef POCKETOS_TSL2561_DRIVER_H
#define POCKETOS_TSL2561_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_TSL2561_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// TSL2561 valid I2C addresses
#define TSL2561_ADDR_COUNT 3
const uint8_t TSL2561_VALID_ADDRESSES[TSL2561_ADDR_COUNT] = { 0x29, 0x39, 0x49 };

// TSL2561 measurement data
struct TSL2561Data {
    float lux;
    uint16_t broadband;
    uint16_t ir;
    bool valid;
    
    TSL2561Data() : lux(0), broadband(0), ir(0), valid(false) {}
};

// TSL2561 Device Driver
class TSL2561Driver {
public:
    TSL2561Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    TSL2561Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "tsl2561"; }
    String getDriverTier() const { return POCKETOS_TSL2561_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = TSL2561_ADDR_COUNT;
        return TSL2561_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < TSL2561_ADDR_COUNT; i++) {
            if (TSL2561_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_TSL2561_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    uint8_t gain;
    uint8_t integrationTime;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Lux calculation
    float calculateLux(uint16_t broadband, uint16_t ir);
};

} // namespace PocketOS

#endif // POCKETOS_TSL2561_DRIVER_H
