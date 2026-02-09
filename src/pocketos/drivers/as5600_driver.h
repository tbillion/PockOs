#ifndef POCKETOS_AS5600_DRIVER_H
#define POCKETOS_AS5600_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_AS5600_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define AS5600_ADDR_COUNT 1
const uint8_t AS5600_VALID_ADDRESSES[AS5600_ADDR_COUNT] = { 0x36 };

struct AS5600Data {
    uint16_t angle;
    uint16_t raw_angle;
    uint8_t status;
    bool valid;
    
    AS5600Data() : angle(0), raw_angle(0), status(0), valid(false) {}
};

class AS5600Driver {
public:
    AS5600Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    AS5600Data readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "as5600"; }
    String getDriverTier() const { return POCKETOS_AS5600_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = AS5600_ADDR_COUNT;
        return AS5600_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x36;
    }
    
#if POCKETOS_AS5600_ENABLE_REGISTER_ACCESS
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
    bool readWord(uint8_t reg, uint16_t* value);
};

} // namespace PocketOS

#endif
