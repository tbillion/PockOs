#ifndef POCKETOS_FT6206_DRIVER_H
#define POCKETOS_FT6206_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_FT6206_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define FT6206_ADDR_COUNT 1
const uint8_t FT6206_VALID_ADDRESSES[FT6206_ADDR_COUNT] = { 0x38 };

struct FT6206TouchPoint {
    uint16_t x;
    uint16_t y;
    uint8_t event;
    bool valid;
    
    FT6206TouchPoint() : x(0), y(0), event(0), valid(false) {}
};

struct FT6206Data {
    uint8_t touches;
    FT6206TouchPoint point1;
    FT6206TouchPoint point2;
    bool valid;
    
    FT6206Data() : touches(0), valid(false) {}
};

class FT6206Driver {
public:
    FT6206Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    FT6206Data readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ft6206"; }
    String getDriverTier() const { return POCKETOS_FT6206_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = FT6206_ADDR_COUNT;
        return FT6206_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x38;
    }
    
#if POCKETOS_FT6206_ENABLE_REGISTER_ACCESS
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
    bool readBlock(uint8_t reg, uint8_t* buffer, size_t length);
};

} // namespace PocketOS

#endif
