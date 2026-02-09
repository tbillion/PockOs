#ifndef POCKETOS_DRV2605_DRIVER_H
#define POCKETOS_DRV2605_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define DRV2605_ADDR_COUNT 1
const uint8_t DRV2605_VALID_ADDRESSES[DRV2605_ADDR_COUNT] = { 0x5A };

class DRV2605Driver {
public:
    DRV2605Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    bool playEffect(uint8_t effect);
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "drv2605"; }
    String getDriverTier() const { return POCKETOS_DRV2605_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = DRV2605_ADDR_COUNT;
        return DRV2605_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return (addr == DRV2605_VALID_ADDRESSES[0]);
    }
    
#if POCKETOS_DRV2605_ENABLE_REGISTER_ACCESS
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
