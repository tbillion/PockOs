#ifndef POCKETOS_HT16K33_DRIVER_H
#define POCKETOS_HT16K33_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define HT16K33_ADDR_COUNT 8
const uint8_t HT16K33_VALID_ADDRESSES[HT16K33_ADDR_COUNT] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77
};

class HT16K33Driver {
public:
    HT16K33Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    bool setPWM(uint8_t channel, uint16_t value);
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ht16k33"; }
    String getDriverTier() const { return POCKETOS_HT16K33_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = HT16K33_ADDR_COUNT;
        return HT16K33_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < HT16K33_ADDR_COUNT; i++) {
            if (HT16K33_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_HT16K33_ENABLE_REGISTER_ACCESS
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
