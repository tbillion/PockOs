#ifndef POCKETOS_TCA9546A_DRIVER_H
#define POCKETOS_TCA9546A_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define TCA9546A_ADDR_COUNT 8
const uint8_t TCA9546A_VALID_ADDRESSES[TCA9546A_ADDR_COUNT] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };

// TCA switch - no data to read

class TCA9546ADriver {
public:
    TCA9546ADriver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    bool selectChannel(uint8_t channel);
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "tca9546a"; }
    String getDriverTier() const { return POCKETOS_TCA9546A_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = TCA9546A_ADDR_COUNT;
        return TCA9546A_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < TCA9546A_ADDR_COUNT; i++) {
            if (TCA9546A_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_TCA9546A_ENABLE_REGISTER_ACCESS
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
