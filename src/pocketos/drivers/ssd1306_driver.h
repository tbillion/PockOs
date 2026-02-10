#ifndef POCKETOS_SSD1306_DRIVER_H
#define POCKETOS_SSD1306_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define SSD1306_ADDR_COUNT 2
const uint8_t SSD1306_VALID_ADDRESSES[SSD1306_ADDR_COUNT] = { 0x3C, 0x3D };

struct SSD1306Data {
    bool display_on;
    bool valid;
    
    SSD1306Data() : display_on(false), valid(false) {}
};

class SSD1306Driver {
public:
    SSD1306Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    bool writeCommand(uint8_t cmd);
    bool displayOn();
    bool displayOff();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ssd1306"; }
    String getDriverTier() const { return POCKETOS_SSD1306_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = SSD1306_ADDR_COUNT;
        return SSD1306_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < SSD1306_ADDR_COUNT; i++) {
            if (SSD1306_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_SSD1306_ENABLE_REGISTER_ACCESS
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
