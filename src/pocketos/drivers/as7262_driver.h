#ifndef POCKETOS_AS7262_DRIVER_H
#define POCKETOS_AS7262_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

// AS7262 valid I2C addresses
#define AS7262_ADDR_COUNT 1
const uint8_t AS7262_VALID_ADDRESSES[AS7262_ADDR_COUNT] = { 0x49 };

// AS7262 spectral data (6 channels - visible spectrum)
struct AS7262Data {
    uint16_t violet;   // 450nm
    uint16_t blue;     // 500nm
    uint16_t green;    // 550nm
    uint16_t yellow;   // 570nm
    uint16_t orange;   // 600nm
    uint16_t red;      // 650nm
    bool valid;
    
    AS7262Data() : violet(0), blue(0), green(0), yellow(0), orange(0), red(0), valid(false) {}
};

// AS7262 Device Driver
class AS7262Driver {
public:
    AS7262Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read spectral data
    AS7262Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "as7262"; }
    String getDriverTier() const { return POCKETOS_AS7262_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = AS7262_ADDR_COUNT;
        return AS7262_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < AS7262_ADDR_COUNT; i++) {
            if (AS7262_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_AS7262_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    // I2C communication helpers
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_AS7262_DRIVER_H
