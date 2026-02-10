#ifndef POCKETOS_PN532_DRIVER_H
#define POCKETOS_PN532_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define PN532_ADDR_COUNT 1
const uint8_t PN532_VALID_ADDRESSES[PN532_ADDR_COUNT] = { 0x24 };

struct PN532Data {
    uint8_t uid[7];
    uint8_t uid_len;
    bool valid;
    
    PN532Data() : uid_len(0), valid(false) { memset(uid, 0, sizeof(uid)); }
};

class PN532Driver {
public:
    PN532Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    PN532Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pn532"; }
    String getDriverTier() const { return POCKETOS_PN532_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = PN532_ADDR_COUNT;
        return PN532_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < PN532_ADDR_COUNT; i++) {
            if (PN532_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_PN532_ENABLE_REGISTER_ACCESS
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
