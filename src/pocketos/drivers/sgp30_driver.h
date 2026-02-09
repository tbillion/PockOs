#ifndef POCKETOS_SGP30_DRIVER_H
#define POCKETOS_SGP30_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_SGP30_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define SGP30_ADDR_COUNT 1
const uint8_t SGP30_VALID_ADDRESSES[SGP30_ADDR_COUNT] = { 0x58 };

struct SGP30Data {
    uint16_t tvoc;
    uint16_t eco2;
    bool valid;
    
    SGP30Data() : tvoc(0), eco2(0), valid(false) {}
};

class SGP30Driver {
public:
    SGP30Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    SGP30Data readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "sgp30"; }
    String getDriverTier() const { return POCKETOS_SGP30_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = SGP30_ADDR_COUNT;
        return SGP30_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x58;
    }
    
#if POCKETOS_SGP30_ENABLE_REGISTER_ACCESS
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool sendCommand(uint16_t command);
    bool readResponse(uint8_t* buffer, size_t length);
};

} // namespace PocketOS

#endif
