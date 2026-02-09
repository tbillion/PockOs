#ifndef POCKETOS_AS7263_DRIVER_H
#define POCKETOS_AS7263_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

// AS7263 valid I2C addresses
#define AS7263_ADDR_COUNT 1
const uint8_t AS7263_VALID_ADDRESSES[AS7263_ADDR_COUNT] = { 0x49 };

// AS7263 spectral data (6 channels - NIR spectrum)
struct AS7263Data {
    uint16_t r;  // 610nm
    uint16_t s;  // 680nm
    uint16_t t;  // 730nm
    uint16_t u;  // 760nm
    uint16_t v;  // 810nm
    uint16_t w;  // 860nm
    bool valid;
    
    AS7263Data() : r(0), s(0), t(0), u(0), v(0), w(0), valid(false) {}
};

// AS7263 Device Driver
class AS7263Driver {
public:
    AS7263Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read spectral data
    AS7263Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "as7263"; }
    String getDriverTier() const { return POCKETOS_AS7263_TIER_NAME; }
    
    // Address enumeration
    static const uint8_t* validAddresses(size_t& count) {
        count = AS7263_ADDR_COUNT;
        return AS7263_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < AS7263_ADDR_COUNT; i++) {
            if (AS7263_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_AS7263_ENABLE_REGISTER_ACCESS
    // Tier 2: Register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
};

} // namespace PocketOS

#endif
