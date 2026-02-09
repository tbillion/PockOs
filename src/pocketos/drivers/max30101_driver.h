#ifndef POCKETOS_MAX30101_DRIVER_H
#define POCKETOS_MAX30101_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MAX30101_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define MAX30101_ADDR_COUNT 1
const uint8_t MAX30101_VALID_ADDRESSES[MAX30101_ADDR_COUNT] = { 0x57 };

struct MAX30101Data {
    uint32_t red;
    uint32_t ir;
    uint32_t green;
    bool valid;
    
    MAX30101Data() : red(0), ir(0), green(0), valid(false) {}
};

class MAX30101Driver {
public:
    MAX30101Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    MAX30101Data readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "max30101"; }
    String getDriverTier() const { return POCKETOS_MAX30101_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = MAX30101_ADDR_COUNT;
        return MAX30101_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x57;
    }
    
#if POCKETOS_MAX30101_ENABLE_REGISTER_ACCESS
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
    uint32_t readFIFO();
};

} // namespace PocketOS

#endif
