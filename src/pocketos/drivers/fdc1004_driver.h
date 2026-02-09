#ifndef POCKETOS_FDC1004_DRIVER_H
#define POCKETOS_FDC1004_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define FDC1004_ADDR_COUNT 1
const uint8_t FDC1004_VALID_ADDRESSES[FDC1004_ADDR_COUNT] = { 0x50 };

struct FDC1004Data {
    uint16_t value;
    bool valid;
    
    FDC1004Data() : value(0), valid(false) {}
};

class FDC1004Driver {
public:
    FDC1004Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    FDC1004Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "fdc1004"; }
    String getDriverTier() const { return POCKETOS_FDC1004_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = FDC1004_ADDR_COUNT;
        return FDC1004_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < FDC1004_ADDR_COUNT; i++) {
            if (FDC1004_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_FDC1004_ENABLE_REGISTER_ACCESS
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
