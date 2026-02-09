#ifndef POCKETOS_SGP40_DRIVER_H
#define POCKETOS_SGP40_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_SGP40_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define SGP40_ADDR_COUNT 1
const uint8_t SGP40_VALID_ADDRESSES[SGP40_ADDR_COUNT] = { 0x59 };

struct SGP40Data {
    uint16_t voc_raw;
    int32_t voc_index;
    bool valid;
    
    SGP40Data() : voc_raw(0), voc_index(0), valid(false) {}
};

class SGP40Driver {
public:
    SGP40Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    SGP40Data readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "sgp40"; }
    String getDriverTier() const { return POCKETOS_SGP40_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = SGP40_ADDR_COUNT;
        return SGP40_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x59;
    }
    
#if POCKETOS_SGP40_ENABLE_REGISTER_ACCESS
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool sendCommand(uint16_t command, const uint8_t* params, size_t paramLen);
    bool readResponse(uint8_t* buffer, size_t length);
};

} // namespace PocketOS

#endif
