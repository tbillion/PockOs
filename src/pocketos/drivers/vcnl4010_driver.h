#ifndef POCKETOS_VCNL4010_DRIVER_H
#define POCKETOS_VCNL4010_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_VCNL4010_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// VCNL4010 valid I2C addresses
#define VCNL4010_ADDR_COUNT 1
const uint8_t VCNL4010_VALID_ADDRESSES[VCNL4010_ADDR_COUNT] = { 0x13 };

// VCNL4010 measurement data
struct VCNL4010Data {
    uint16_t proximity;
    float ambient;
    bool valid;
    
    VCNL4010Data() : proximity(0), ambient(0), valid(false) {}
};

// VCNL4010 Device Driver
class VCNL4010Driver {
public:
    VCNL4010Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    VCNL4010Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "vcnl4010"; }
    String getDriverTier() const { return POCKETOS_VCNL4010_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = VCNL4010_ADDR_COUNT;
        return VCNL4010_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < VCNL4010_ADDR_COUNT; i++) {
            if (VCNL4010_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_VCNL4010_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_VCNL4010_DRIVER_H
