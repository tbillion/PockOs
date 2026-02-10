#ifndef POCKETOS_MCP3421_DRIVER_H
#define POCKETOS_MCP3421_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define MCP3421_ADDR_COUNT 8
const uint8_t MCP3421_VALID_ADDRESSES[MCP3421_ADDR_COUNT] = { 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F };

struct MCP3421Data {
    uint16_t value;
    bool valid;
    
    MCP3421Data() : value(0), valid(false) {}
};

class MCP3421Driver {
public:
    MCP3421Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    MCP3421Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mcp3421"; }
    String getDriverTier() const { return POCKETOS_MCP3421_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = MCP3421_ADDR_COUNT;
        return MCP3421_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MCP3421_ADDR_COUNT; i++) {
            if (MCP3421_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MCP3421_ENABLE_REGISTER_ACCESS
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
