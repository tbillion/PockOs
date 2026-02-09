#ifndef POCKETOS_TCS34725_DRIVER_H
#define POCKETOS_TCS34725_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_TCS34725_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// TCS34725 valid I2C addresses
#define TCS34725_ADDR_COUNT 1
const uint8_t TCS34725_VALID_ADDRESSES[TCS34725_ADDR_COUNT] = { 0x29 };

// TCS34725 color data
struct TCS34725Data {
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t c;
    float lux;
    uint16_t colorTemp;
    bool valid;
    
    TCS34725Data() : r(0), g(0), b(0), c(0), lux(0), colorTemp(0), valid(false) {}
};

// TCS34725 Device Driver
class TCS34725Driver {
public:
    TCS34725Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    TCS34725Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "tcs34725"; }
    String getDriverTier() const { return POCKETOS_TCS34725_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = TCS34725_ADDR_COUNT;
        return TCS34725_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < TCS34725_ADDR_COUNT; i++) {
            if (TCS34725_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_TCS34725_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    uint8_t integrationTime;
    uint8_t gain;
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t len);
    
    // Conversion functions
    void calculateLuxAndCCT(uint16_t r, uint16_t g, uint16_t b, uint16_t c);
};

} // namespace PocketOS

#endif // POCKETOS_TCS34725_DRIVER_H
