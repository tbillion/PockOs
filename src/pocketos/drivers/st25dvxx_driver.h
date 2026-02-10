#ifndef POCKETOS_ST25DVXX_DRIVER_H
#define POCKETOS_ST25DVXX_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define ST25DVXX_ADDR_COUNT 2
const uint8_t ST25DVXX_VALID_ADDRESSES[ST25DVXX_ADDR_COUNT] = { 0x53, 0x57 };

struct ST25DVxxData {
    uint8_t uid[7];
    uint8_t uid_len;
    bool valid;
    
    ST25DVxxData() : uid_len(0), valid(false) { memset(uid, 0, sizeof(uid)); }
};

class ST25DVxxDriver {
public:
    ST25DVxxDriver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    ST25DVxxData readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "st25dvxx"; }
    String getDriverTier() const { return POCKETOS_ST25DVXX_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = ST25DVXX_ADDR_COUNT;
        return ST25DVXX_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < ST25DVXX_ADDR_COUNT; i++) {
            if (ST25DVXX_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_ST25DVXX_ENABLE_REGISTER_ACCESS
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
