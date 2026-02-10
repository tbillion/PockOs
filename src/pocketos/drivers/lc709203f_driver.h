#ifndef POCKETOS_LC709203F_DRIVER_H
#define POCKETOS_LC709203F_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define LC709203F_ADDR_COUNT 1
const uint8_t LC709203F_VALID_ADDRESSES[LC709203F_ADDR_COUNT] = { 0x0B };

struct LC709203FData {
    float voltage;      // V
    float percentage;   // %
    bool valid;
    
    LC709203FData() : voltage(0), percentage(0), valid(false) {}
};

class LC709203FDriver {
public:
    LC709203FDriver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    LC709203FData readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "lc709203f"; }
    String getDriverTier() const { return POCKETOS_LC709203F_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = LC709203F_ADDR_COUNT;
        return LC709203F_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return (addr == LC709203F_VALID_ADDRESSES[0]);
    }
    
#if POCKETOS_LC709203F_ENABLE_REGISTER_ACCESS
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t* value);
};

} // namespace PocketOS

#endif
