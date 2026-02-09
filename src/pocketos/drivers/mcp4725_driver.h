#ifndef POCKETOS_MCP4725_DRIVER_H
#define POCKETOS_MCP4725_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define MCP4725_ADDR_COUNT 8
const uint8_t MCP4725_VALID_ADDRESSES[MCP4725_ADDR_COUNT] = { 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 };

struct MCP4725Data {
    uint16_t value;
    bool valid;
    
    MCP4725Data() : value(0), valid(false) {}
};

class MCP4725Driver {
public:
    MCP4725Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    MCP4725Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mcp4725"; }
    String getDriverTier() const { return POCKETOS_MCP4725_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = MCP4725_ADDR_COUNT;
        return MCP4725_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MCP4725_ADDR_COUNT; i++) {
            if (MCP4725_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MCP4725_ENABLE_REGISTER_ACCESS
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
