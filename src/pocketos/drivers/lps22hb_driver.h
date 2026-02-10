#ifndef POCKETOS_LPS22HB_DRIVER_H
#define POCKETOS_LPS22HB_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_LPS22HB_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// LPS22HB valid I2C addresses
#define LPS22HB_ADDR_COUNT 2
const uint8_t LPS22HB_VALID_ADDRESSES[LPS22HB_ADDR_COUNT] = { 0x5C, 0x5D };

// LPS22HB measurement data
struct LPS22HBData {
    float temperature;  // Celsius
    float pressure;     // hPa
    bool valid;
    
    LPS22HBData() : temperature(0), pressure(0), valid(false) {}
};

// LPS22HB Device Driver
class LPS22HBDriver {
public:
    LPS22HBDriver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    LPS22HBData readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "lps22hb"; }
    String getDriverTier() const { return POCKETOS_LPS22HB_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = LPS22HB_ADDR_COUNT;
        return LPS22HB_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < LPS22HB_ADDR_COUNT; i++) {
            if (LPS22HB_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_LPS22HB_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_LPS22HB_DRIVER_H
