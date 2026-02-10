#ifndef POCKETOS_AT24CXX_DRIVER_H
#define POCKETOS_AT24CXX_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define AT24CXX_ADDR_COUNT 8
const uint8_t AT24CXX_VALID_ADDRESSES[AT24CXX_ADDR_COUNT] = { 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 };

struct AT24CxxData {
    uint16_t value;
    bool valid;
    
    AT24CxxData() : value(0), valid(false) {}
};

class AT24CxxDriver {
public:
    AT24CxxDriver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    AT24CxxData readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "at24cxx"; }
    String getDriverTier() const { return POCKETOS_AT24CXX_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = AT24CXX_ADDR_COUNT;
        return AT24CXX_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < AT24CXX_ADDR_COUNT; i++) {
            if (AT24CXX_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_AT24CXX_ENABLE_REGISTER_ACCESS
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
