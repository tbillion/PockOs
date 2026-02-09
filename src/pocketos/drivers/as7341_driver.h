#ifndef POCKETOS_AS7341_DRIVER_H
#define POCKETOS_AS7341_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define AS7341_ADDR_COUNT 1
const uint8_t AS7341_VALID_ADDRESSES[AS7341_ADDR_COUNT] = { 0x39 };

struct AS7341Data {
    uint16_t ch415nm;  // F1
    uint16_t ch445nm;  // F2
    uint16_t ch480nm;  // F3
    uint16_t ch515nm;  // F4
    uint16_t ch555nm;  // F5
    uint16_t ch590nm;  // F6
    uint16_t ch630nm;  // F7
    uint16_t ch680nm;  // F8
    uint16_t clear;
    uint16_t nir;
    bool valid;
    
    AS7341Data() : ch415nm(0), ch445nm(0), ch480nm(0), ch515nm(0), ch555nm(0), 
                   ch590nm(0), ch630nm(0), ch680nm(0), clear(0), nir(0), valid(false) {}
};

class AS7341Driver {
public:
    AS7341Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    AS7341Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "as7341"; }
    String getDriverTier() const { return POCKETOS_AS7341_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = AS7341_ADDR_COUNT;
        return AS7341_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return (addr == AS7341_VALID_ADDRESSES[0]);
    }
    
#if POCKETOS_AS7341_ENABLE_REGISTER_ACCESS
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
