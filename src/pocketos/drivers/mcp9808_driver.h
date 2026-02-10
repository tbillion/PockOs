#ifndef POCKETOS_MCP9808_DRIVER_H
#define POCKETOS_MCP9808_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// MCP9808 valid I2C addresses
#define MCP9808_ADDR_COUNT 8
const uint8_t MCP9808_VALID_ADDRESSES[MCP9808_ADDR_COUNT] = { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };

// MCP9808 measurement data
struct MCP9808Data {
    float temperature;  // Celsius
    bool valid;
    
    MCP9808Data() : temperature(0), valid(false) {}
};

// MCP9808 Device Driver (Tier 0/1)
class MCP9808Driver {
public:
    MCP9808Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    MCP9808Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mcp9808"; }
    String getDriverTier() const { return POCKETOS_MCP9808_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = MCP9808_ADDR_COUNT;
        return MCP9808_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MCP9808_ADDR_COUNT; i++) {
            if (MCP9808_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MCP9808_ENABLE_CONFIGURATION
    // Configuration (Tier 1 only)
    bool setResolution(uint8_t resolution);
    bool shutdown(bool enable);
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_MCP9808_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool readRegister(uint8_t reg, uint8_t* buffer, size_t len);
    bool writeRegister(uint8_t reg, uint16_t value);
};

} // namespace PocketOS

#endif // POCKETOS_MCP9808_DRIVER_H
