#ifndef POCKETOS_MPR121_DRIVER_H
#define POCKETOS_MPR121_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MPR121_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define MPR121_ADDR_COUNT 4
const uint8_t MPR121_VALID_ADDRESSES[MPR121_ADDR_COUNT] = { 0x5A, 0x5B, 0x5C, 0x5D };

struct MPR121Data {
    uint16_t touched;
    uint16_t filtered[12];
    bool valid;
    
    MPR121Data() : touched(0), valid(false) {
        for (int i = 0; i < 12; i++) filtered[i] = 0;
    }
};

class MPR121Driver {
public:
    MPR121Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    MPR121Data readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mpr121"; }
    String getDriverTier() const { return POCKETOS_MPR121_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = MPR121_ADDR_COUNT;
        return MPR121_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MPR121_ADDR_COUNT; i++) {
            if (MPR121_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MPR121_ENABLE_REGISTER_ACCESS
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool readRegister(uint8_t reg, uint8_t* value);
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readWord(uint8_t reg, uint16_t* value);
};

} // namespace PocketOS

#endif
