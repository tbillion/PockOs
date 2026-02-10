#ifndef POCKETOS_RV3028_DRIVER_H
#define POCKETOS_RV3028_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_RV3028_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// RV3028 valid I2C address
#define RV3028_ADDR_COUNT 1
const uint8_t RV3028_VALID_ADDRESSES[RV3028_ADDR_COUNT] = { 0x52 };

// RV3028 DateTime structure
struct RV3028DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    bool valid;
    
    RV3028DateTime() : year(2000), month(1), day(1), hour(0), minute(0), second(0), dayOfWeek(0), valid(false) {}
};

// RV3028 Alarm structure
struct RV3028Alarm {
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    bool dayEnable;
    bool hourEnable;
    bool minuteEnable;
    bool enabled;
    
    RV3028Alarm() : day(0), hour(0), minute(0), 
                    dayEnable(false), hourEnable(false), minuteEnable(false), 
                    enabled(false) {}
};

// RV3028 Ultra-Low Power RTC Driver
class RV3028Driver {
public:
    RV3028Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Time operations
    RV3028DateTime readDateTime();
    bool setDateTime(const RV3028DateTime& dt);
    
    // Unix time support
    uint32_t readUnixTime();
    bool setUnixTime(uint32_t unixTime);
    
#if POCKETOS_RV3028_ENABLE_ALARM_FEATURES
    // Alarm operations
    bool setAlarm(const RV3028Alarm& alarm);
    RV3028Alarm getAlarm();
    bool checkAlarmFlag();
    bool clearAlarmFlag();
    
    // Periodic countdown timer
    bool setCountdownTimer(uint16_t ticks, uint8_t source, bool repeat);  // source: 0=4096Hz, 1=64Hz, 2=1Hz, 3=1/60Hz
    bool enableTimer(bool enable);
    bool checkTimerFlag();
    bool clearTimerFlag();
    
    // Periodic time update interrupt
    bool setPeriodicTimeUpdate(uint8_t period);  // period: 0=1/sec, 1=1/min, 2=1/hour, 3=1/day, 4=1/week, 5=1/month, 6=1/year
    bool enablePeriodicUpdate(bool enable);
    bool checkUpdateFlag();
    bool clearUpdateFlag();
    
    // Clock output control (CLKOUT)
    bool setClockOutput(bool enable, uint8_t freq);  // freq: 0=32.768kHz, 1=8.192kHz, 2=1.024kHz, 3=64Hz, 4=32Hz, 5=1Hz
    
    // EEPROM access (43 bytes user EEPROM)
    bool readEEPROM(uint8_t address, uint8_t* data, uint8_t length);
    bool writeEEPROM(uint8_t address, const uint8_t* data, uint8_t length);
    
    // Trickle charger
    bool setTrickleCharger(uint8_t resistor, uint8_t diode);  // resistor: 0=disabled, 1=1.5kΩ, 2=3kΩ, 3=9kΩ, 4=15kΩ
    
    // Offset calibration (2's complement, -64 to +63)
    bool setOffset(int8_t offset);
    int8_t getOffset();
    
    // Automatic backup switchover
    bool enableBackupSwitchover(bool enable);
    
    // Battery low detection
    bool checkBatteryLow();
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "rv3028"; }
    String getDriverTier() const { return POCKETOS_RV3028_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = RV3028_ADDR_COUNT;
        return RV3028_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < RV3028_ADDR_COUNT; i++) {
            if (RV3028_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_RV3028_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_RV3028_DRIVER_H
