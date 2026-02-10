#ifndef POCKETOS_BH1750_DRIVER_H
#define POCKETOS_BH1750_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_BH1750_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// BH1750 valid I2C addresses
#define BH1750_ADDR_COUNT 2
const uint8_t BH1750_VALID_ADDRESSES[BH1750_ADDR_COUNT] = { 0x23, 0x5C };

// BH1750 measurement data
struct BH1750Data {
    float lux;
    bool valid;
    
    BH1750Data() : lux(0), valid(false) {}
};

// BH1750 Device Driver
class BH1750Driver {
public:
    BH1750Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    BH1750Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "bh1750"; }
    String getDriverTier() const { return POCKETOS_BH1750_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = BH1750_ADDR_COUNT;
        return BH1750_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < BH1750_ADDR_COUNT; i++) {
            if (BH1750_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_BH1750_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    uint8_t mode;
    
    // I2C communication
    bool writeCommand(uint8_t cmd);
    bool readData(uint16_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_BH1750_DRIVER_H
