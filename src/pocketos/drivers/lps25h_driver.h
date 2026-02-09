#ifndef POCKETOS_LPS25H_DRIVER_H
#define POCKETOS_LPS25H_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_LPS25H_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// LPS25H valid I2C addresses
#define LPS25H_ADDR_COUNT 2
const uint8_t LPS25H_VALID_ADDRESSES[LPS25H_ADDR_COUNT] = { 0x5C, 0x5D };

// LPS25H measurement data
struct LPS25HData {
    float temperature;  // Celsius
    float pressure;     // hPa
    bool valid;
    
    LPS25HData() : temperature(0), pressure(0), valid(false) {}
};

// LPS25H Device Driver
class LPS25HDriver {
public:
    LPS25HDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    LPS25HData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "lps25h"; }
    String getDriverTier() const { return POCKETOS_LPS25H_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = LPS25H_ADDR_COUNT;
        return LPS25H_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < LPS25H_ADDR_COUNT; i++) {
            if (LPS25H_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_LPS25H_ENABLE_REGISTER_ACCESS
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
};

} // namespace PocketOS

#endif // POCKETOS_LPS25H_DRIVER_H
