#ifndef POCKETOS_PCF8523_DRIVER_H
#define POCKETOS_PCF8523_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_PCF8523_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// PCF8523 valid I2C address
#define PCF8523_ADDR_COUNT 1
const uint8_t PCF8523_VALID_ADDRESSES[PCF8523_ADDR_COUNT] = { 0x68 };

// PCF8523 DateTime structure
struct PCF8523DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    bool valid;
    
    PCF8523DateTime() : year(2000), month(1), day(1), hour(0), minute(0), second(0), dayOfWeek(0), valid(false) {}
};

// PCF8523 Alarm structure
struct PCF8523Alarm {
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    bool dayEnable;
    bool hourEnable;
    bool minuteEnable;
    bool enabled;
    
    PCF8523Alarm() : day(0), hour(0), minute(0), 
                     dayEnable(false), hourEnable(false), minuteEnable(false), 
                     enabled(false) {}
};

// PCF8523 Low-Power RTC Driver
class PCF8523Driver {
public:
    PCF8523Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Time operations
    PCF8523DateTime readDateTime();
    bool setDateTime(const PCF8523DateTime& dt);
    
#if POCKETOS_PCF8523_ENABLE_ALARM_FEATURES
    // Alarm operations
    bool setAlarm(const PCF8523Alarm& alarm);
    PCF8523Alarm getAlarm();
    bool checkAlarmFlag();
    bool clearAlarmFlag();
    
    // Timer operations (countdown timer)
    bool setCountdownTimer(uint8_t source, uint8_t value, bool repeat);  // source: 0=4.096kHz, 1=64Hz, 2=1Hz, 3=1/60Hz
    bool enableTimer(bool enable);
    bool checkTimerFlag();
    bool clearTimerFlag();
    
    // Clock output control
    bool setClockOutput(bool enable, uint8_t freq);  // freq: 0=32.768kHz, 1=16.384kHz, 2=8.192kHz, 3=4.096kHz, 4=1.024kHz, 5=32Hz, 6=1Hz, 7=disabled
    
    // Offset calibration (mode: 0=slow, 1=fast; offset: 0-63)
    bool setOffset(uint8_t mode, uint8_t offset);
    
    // Battery switch-over mode
    bool setBatteryMode(uint8_t mode);  // 0=standard, 1=direct, 2=low power
    
    // Capacitor selection
    bool setCapacitorSelection(uint8_t cap);  // 0=7pF, 1=12.5pF
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pcf8523"; }
    String getDriverTier() const { return POCKETOS_PCF8523_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = PCF8523_ADDR_COUNT;
        return PCF8523_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < PCF8523_ADDR_COUNT; i++) {
            if (PCF8523_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_PCF8523_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_PCF8523_DRIVER_H
