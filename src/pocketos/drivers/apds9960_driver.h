#ifndef POCKETOS_APDS9960_DRIVER_H
#define POCKETOS_APDS9960_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_APDS9960_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// APDS9960 valid I2C addresses
#define APDS9960_ADDR_COUNT 1
const uint8_t APDS9960_VALID_ADDRESSES[APDS9960_ADDR_COUNT] = { 0x39 };

// APDS9960 gesture direction
enum class GestureDirection {
    NONE = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NEAR,
    FAR
};

// APDS9960 color data
struct APDS9960ColorData {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t clear;
    bool valid;
    
    APDS9960ColorData() : red(0), green(0), blue(0), clear(0), valid(false) {}
};

// APDS9960 proximity data
struct APDS9960ProximityData {
    uint8_t proximity;
    bool valid;
    
    APDS9960ProximityData() : proximity(0), valid(false) {}
};

class APDS9960Driver {
public:
    APDS9960Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    APDS9960ColorData readColor();
    APDS9960ProximityData readProximity();
    GestureDirection readGesture();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "apds9960"; }
    String getDriverTier() const { return POCKETOS_APDS9960_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = APDS9960_ADDR_COUNT;
        return APDS9960_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < APDS9960_ADDR_COUNT; i++) {
            if (APDS9960_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_APDS9960_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    bool gestureMode;
    
    bool readRegister(uint8_t reg, uint8_t* value);
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readBlock(uint8_t reg, uint8_t* buffer, size_t length);
    bool enableGesture();
    bool disableGesture();
};

} // namespace PocketOS

#endif
