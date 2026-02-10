#ifndef POCKETOS_CCS811_DRIVER_H
#define POCKETOS_CCS811_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_CCS811_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// CCS811 valid I2C addresses
#define CCS811_ADDR_COUNT 2
const uint8_t CCS811_VALID_ADDRESSES[CCS811_ADDR_COUNT] = { 0x5A, 0x5B };

// CCS811 air quality data
struct CCS811Data {
    uint16_t eco2;      // eCO2 in ppm
    uint16_t tvoc;      // TVOC in ppb
    bool valid;
    
    CCS811Data() : eco2(0), tvoc(0), valid(false) {}
};

class CCS811Driver {
public:
    CCS811Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    CCS811Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ccs811"; }
    String getDriverTier() const { return POCKETOS_CCS811_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = CCS811_ADDR_COUNT;
        return CCS811_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < CCS811_ADDR_COUNT; i++) {
            if (CCS811_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_CCS811_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool readRegister(uint8_t reg, uint8_t* buffer, size_t length);
    bool writeRegister(uint8_t reg);
    bool checkDataReady();
};

} // namespace PocketOS

#endif
