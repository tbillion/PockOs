#ifndef POCKETOS_DS1307_DRIVER_H
#define POCKETOS_DS1307_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_DS1307_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// DS1307 valid I2C address
#define DS1307_ADDR_COUNT 1
const uint8_t DS1307_VALID_ADDRESSES[DS1307_ADDR_COUNT] = { 0x68 };

// DS1307 DateTime structure
struct DS1307DateTime {
    uint16_t year;    // Full year (e.g., 2024)
    uint8_t month;    // 1-12
    uint8_t day;      // 1-31
    uint8_t hour;     // 0-23
    uint8_t minute;   // 0-59
    uint8_t second;   // 0-59
    uint8_t dayOfWeek; // 1-7 (1=Sunday)
    bool valid;
    
    DS1307DateTime() : year(2000), month(1), day(1), hour(0), minute(0), second(0), dayOfWeek(1), valid(false) {}
};

// DS1307 RTC Driver
class DS1307Driver {
public:
    DS1307Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Time operations
    DS1307DateTime readDateTime();
    bool setDateTime(const DS1307DateTime& dt);
    
#if POCKETOS_DS1307_ENABLE_ALARM_FEATURES
    // SRAM access (56 bytes at 0x08-0x3F)
    bool readSRAM(uint8_t offset, uint8_t* data, uint8_t length);
    bool writeSRAM(uint8_t offset, const uint8_t* data, uint8_t length);
    
    // Square wave control
    bool setSquareWave(bool enable, uint8_t rate);  // rate: 0=1Hz, 1=4.096kHz, 2=8.192kHz, 3=32.768kHz
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ds1307"; }
    String getDriverTier() const { return POCKETOS_DS1307_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = DS1307_ADDR_COUNT;
        return DS1307_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < DS1307_ADDR_COUNT; i++) {
            if (DS1307_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_DS1307_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    // Helper functions
    bool readRegister(uint8_t reg, uint8_t* value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length);
    bool writeRegister(uint8_t reg, uint8_t value);
    bool writeRegisters(uint8_t reg, const uint8_t* buffer, uint8_t length);
    
    // BCD conversion
    uint8_t bcdToDec(uint8_t val);
    uint8_t decToBcd(uint8_t val);
};

} // namespace PocketOS

#endif // POCKETOS_DS1307_DRIVER_H
