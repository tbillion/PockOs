#ifndef POCKETOS_PCF2129_DRIVER_H
#define POCKETOS_PCF2129_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_PCF2129_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// PCF2129 valid I2C address
#define PCF2129_ADDR_COUNT 1
const uint8_t PCF2129_VALID_ADDRESSES[PCF2129_ADDR_COUNT] = { 0x51 };

// PCF2129 DateTime structure
struct PCF2129DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    bool valid;
    
    PCF2129DateTime() : year(2000), month(1), day(1), hour(0), minute(0), second(0), dayOfWeek(0), valid(false) {}
};

// PCF2129 Alarm structure
struct PCF2129Alarm {
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool dayEnable;
    bool hourEnable;
    bool minuteEnable;
    bool secondEnable;
    bool enabled;
    
    PCF2129Alarm() : day(0), hour(0), minute(0), second(0), 
                     dayEnable(false), hourEnable(false), minuteEnable(false), secondEnable(false), 
                     enabled(false) {}
};

// PCF2129 RTC with Alarm Driver
class PCF2129Driver {
public:
    PCF2129Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Time operations
    PCF2129DateTime readDateTime();
    bool setDateTime(const PCF2129DateTime& dt);
    
#if POCKETOS_PCF2129_ENABLE_ALARM_FEATURES
    // Alarm operations
    bool setAlarm(const PCF2129Alarm& alarm);
    PCF2129Alarm getAlarm();
    bool checkAlarmFlag();
    bool clearAlarmFlag();
    
    // Timer operations
    bool setTimer(uint16_t seconds, bool repeat);
    bool enableTimer(bool enable);
    bool checkTimerFlag();
    bool clearTimerFlag();
    
    // Clock output control
    bool setClockOutput(bool enable, uint8_t freq);  // freq: 0=32.768kHz, 1=16.384kHz, 2=8.192kHz, 3=4.096kHz, 4=2.048kHz, 5=1.024kHz, 6=1Hz, 7=disabled
    
    // Timestamp capture
    bool enableTimestamp(bool enable);
    bool getTimestamp(PCF2129DateTime& dt);
    bool clearTimestampFlag();
    
    // Aging offset (trim capacitor: 0-3)
    bool setAgingOffset(uint8_t offset);
    uint8_t getAgingOffset();
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pcf2129"; }
    String getDriverTier() const { return POCKETOS_PCF2129_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = PCF2129_ADDR_COUNT;
        return PCF2129_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < PCF2129_ADDR_COUNT; i++) {
            if (PCF2129_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_PCF2129_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_PCF2129_DRIVER_H
