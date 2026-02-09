#ifndef POCKETOS_DS3231_DRIVER_H
#define POCKETOS_DS3231_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_DS3231_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// DS3231 valid I2C address
#define DS3231_ADDR_COUNT 1
const uint8_t DS3231_VALID_ADDRESSES[DS3231_ADDR_COUNT] = { 0x68 };

// DS3231 DateTime structure
struct DS3231DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    bool valid;
    
    DS3231DateTime() : year(2000), month(1), day(1), hour(0), minute(0), second(0), dayOfWeek(1), valid(false) {}
};

// DS3231 Alarm structure
struct DS3231Alarm {
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t mode;  // Alarm mode bits
    bool enabled;
    
    DS3231Alarm() : day(0), hour(0), minute(0), second(0), mode(0), enabled(false) {}
};

// DS3231 Precision RTC Driver
class DS3231Driver {
public:
    DS3231Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Time operations
    DS3231DateTime readDateTime();
    bool setDateTime(const DS3231DateTime& dt);
    
    // Temperature reading
    float readTemperature();
    
#if POCKETOS_DS3231_ENABLE_ALARM_FEATURES
    // Alarm operations
    bool setAlarm1(const DS3231Alarm& alarm);
    bool setAlarm2(const DS3231Alarm& alarm);
    DS3231Alarm getAlarm1();
    DS3231Alarm getAlarm2();
    bool checkAlarm1();
    bool checkAlarm2();
    bool clearAlarm1();
    bool clearAlarm2();
    
    // Aging offset calibration (-128 to +127)
    bool setAgingOffset(int8_t offset);
    int8_t getAgingOffset();
    
    // Square wave control
    bool setSquareWave(bool enable, uint8_t rate);  // rate: 0=1Hz, 1=1.024kHz, 2=4.096kHz, 3=8.192kHz
    
    // 32kHz output control
    bool enable32kHzOutput(bool enable);
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ds3231"; }
    String getDriverTier() const { return POCKETOS_DS3231_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = DS3231_ADDR_COUNT;
        return DS3231_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < DS3231_ADDR_COUNT; i++) {
            if (DS3231_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_DS3231_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_DS3231_DRIVER_H
