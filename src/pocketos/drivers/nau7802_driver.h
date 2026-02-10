#ifndef POCKETOS_NAU7802_DRIVER_H
#define POCKETOS_NAU7802_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define NAU7802_ADDR_COUNT 2
const uint8_t NAU7802_VALID_ADDRESSES[NAU7802_ADDR_COUNT] = { 0x2A, 0x2B };

struct NAU7802Data {
    int32_t adcValue;
    bool valid;
    
    NAU7802Data() : adcValue(0), valid(false) {}
};

class NAU7802Driver {
public:
    NAU7802Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    NAU7802Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "nau7802"; }
    String getDriverTier() const { return POCKETOS_NAU7802_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = NAU7802_ADDR_COUNT;
        return NAU7802_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < NAU7802_ADDR_COUNT; i++) {
            if (NAU7802_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_NAU7802_ENABLE_REGISTER_ACCESS
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
