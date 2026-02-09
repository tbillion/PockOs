#ifndef POCKETOS_MCP4728_DRIVER_H
#define POCKETOS_MCP4728_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define MCP4728_ADDR_COUNT 8
const uint8_t MCP4728_VALID_ADDRESSES[MCP4728_ADDR_COUNT] = { 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 };

struct MCP4728Data {
    uint16_t value;
    bool valid;
    
    MCP4728Data() : value(0), valid(false) {}
};

class MCP4728Driver {
public:
    MCP4728Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    MCP4728Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mcp4728"; }
    String getDriverTier() const { return POCKETOS_MCP4728_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = MCP4728_ADDR_COUNT;
        return MCP4728_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MCP4728_ADDR_COUNT; i++) {
            if (MCP4728_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MCP4728_ENABLE_REGISTER_ACCESS
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
