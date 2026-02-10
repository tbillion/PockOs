#ifndef POCKETOS_MAG3110_DRIVER_H
#define POCKETOS_MAG3110_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MAG3110_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define MAG3110_ADDR_COUNT 1
const uint8_t MAG3110_VALID_ADDRESSES[MAG3110_ADDR_COUNT] = { 0x0E };

struct MAG3110Data {
    int16_t x;
    int16_t y;
    int16_t z;
    bool valid;
    
    MAG3110Data() : x(0), y(0), z(0), valid(false) {}
};

class MAG3110Driver {
public:
    MAG3110Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    MAG3110Data readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mag3110"; }
    String getDriverTier() const { return POCKETOS_MAG3110_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = MAG3110_ADDR_COUNT;
        return MAG3110_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x0E;
    }
    
#if POCKETOS_MAG3110_ENABLE_REGISTER_ACCESS
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
