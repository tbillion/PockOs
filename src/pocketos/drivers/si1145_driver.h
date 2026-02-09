#ifndef POCKETOS_SI1145_DRIVER_H
#define POCKETOS_SI1145_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_SI1145_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// SI1145 valid I2C addresses
#define SI1145_ADDR_COUNT 1
const uint8_t SI1145_VALID_ADDRESSES[SI1145_ADDR_COUNT] = { 0x60 };

// SI1145 measurement data
struct SI1145Data {
    float uv;
    float visible;
    float ir;
    float uvIndex;
    bool valid;
    
    SI1145Data() : uv(0), visible(0), ir(0), uvIndex(0), valid(false) {}
};

// SI1145 Device Driver
class SI1145Driver {
public:
    SI1145Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    SI1145Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "si1145"; }
    String getDriverTier() const { return POCKETOS_SI1145_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = SI1145_ADDR_COUNT;
        return SI1145_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SI1145_ADDR_COUNT; i++) {
            if (SI1145_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_SI1145_ENABLE_REGISTER_ACCESS
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
    bool writeParam(uint8_t param, uint8_t value);
    
    // Reset and initialization
    bool reset();
};

} // namespace PocketOS

#endif // POCKETOS_SI1145_DRIVER_H
