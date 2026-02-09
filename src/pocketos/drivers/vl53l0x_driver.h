#ifndef POCKETOS_VL53L0X_DRIVER_H
#define POCKETOS_VL53L0X_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define VL53L0X_ADDR_COUNT 1
const uint8_t VL53L0X_VALID_ADDRESSES[VL53L0X_ADDR_COUNT] = { 0x29 };

struct VL53L0XData {
    uint16_t distance_mm;
    bool valid;
    
    VL53L0XData() : distance_mm(0), valid(false) {}
};

class VL53L0XDriver {
public:
    VL53L0XDriver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    VL53L0XData readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "vl53l0x"; }
    String getDriverTier() const { return POCKETOS_VL53L0X_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = VL53L0X_ADDR_COUNT;
        return VL53L0X_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < VL53L0X_ADDR_COUNT; i++) {
            if (VL53L0X_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_VL53L0X_ENABLE_REGISTER_ACCESS
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
