#ifndef POCKETOS_SC16IS750_DRIVER_H
#define POCKETOS_SC16IS750_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define SC16IS750_ADDR_COUNT 8
const uint8_t SC16IS750_VALID_ADDRESSES[SC16IS750_ADDR_COUNT] = { 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };

struct SC16IS750Data {
    uint8_t rx_data;
    bool data_available;
    bool valid;
    
    SC16IS750Data() : rx_data(0), data_available(false), valid(false) {}
};

class SC16IS750Driver {
public:
    SC16IS750Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    SC16IS750Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "sc16is750"; }
    String getDriverTier() const { return POCKETOS_SC16IS750_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = SC16IS750_ADDR_COUNT;
        return SC16IS750_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SC16IS750_ADDR_COUNT; i++) {
            if (SC16IS750_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_SC16IS750_ENABLE_REGISTER_ACCESS
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
