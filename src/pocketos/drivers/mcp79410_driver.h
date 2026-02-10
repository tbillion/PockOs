#ifndef POCKETOS_MCP79410_DRIVER_H
#define POCKETOS_MCP79410_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_MCP79410_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// MCP79410 valid I2C addresses (RTC at 0x6F, EEPROM at 0x57)
#define MCP79410_ADDR_COUNT 1
const uint8_t MCP79410_VALID_ADDRESSES[MCP79410_ADDR_COUNT] = { 0x6F };

// MCP79410 DateTime structure
struct MCP79410DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    bool valid;
    
    MCP79410DateTime() : year(2000), month(1), day(1), hour(0), minute(0), second(0), dayOfWeek(1), valid(false) {}
};

// MCP79410 Alarm structure
struct MCP79410Alarm {
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    uint8_t mask;  // Alarm mask configuration
    bool enabled;
    
    MCP79410Alarm() : month(0), day(0), hour(0), minute(0), second(0), dayOfWeek(0), mask(0), enabled(false) {}
};

// MCP79410 RTC + EEPROM Driver
class MCP79410Driver {
public:
    MCP79410Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Time operations
    MCP79410DateTime readDateTime();
    bool setDateTime(const MCP79410DateTime& dt);
    
#if POCKETOS_MCP79410_ENABLE_ALARM_FEATURES
    // Alarm operations (2 alarms)
    bool setAlarm0(const MCP79410Alarm& alarm);
    bool setAlarm1(const MCP79410Alarm& alarm);
    MCP79410Alarm getAlarm0();
    MCP79410Alarm getAlarm1();
    bool checkAlarm0();
    bool checkAlarm1();
    bool clearAlarm0();
    bool clearAlarm1();
    
    // SRAM access (64 bytes at 0x20-0x5F)
    bool readSRAM(uint8_t offset, uint8_t* data, uint8_t length);
    bool writeSRAM(uint8_t offset, const uint8_t* data, uint8_t length);
    
    // Power-fail timestamp
    bool getPowerFailTimestamp(MCP79410DateTime& dt);
    bool getPowerRestoreTimestamp(MCP79410DateTime& dt);
    
    // Battery backup control
    bool enableBatteryBackup(bool enable);
    
    // Square wave output
    bool setSquareWave(bool enable, uint8_t freq);  // freq: 0=1Hz, 1=4.096kHz, 2=8.192kHz, 3=32.768kHz
    
    // Calibration (trim: -127 to +127)
    bool setCalibration(int8_t trim);
    int8_t getCalibration();
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mcp79410"; }
    String getDriverTier() const { return POCKETOS_MCP79410_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = MCP79410_ADDR_COUNT;
        return MCP79410_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MCP79410_ADDR_COUNT; i++) {
            if (MCP79410_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MCP79410_ENABLE_REGISTER_ACCESS
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

#endif // POCKETOS_MCP79410_DRIVER_H
