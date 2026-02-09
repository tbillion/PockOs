#include "rv3028_driver.h"
#include "../driver_config.h"

#if POCKETOS_RV3028_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// RV3028 Register addresses
#define RV3028_REG_SECONDS        0x00
#define RV3028_REG_MINUTES        0x01
#define RV3028_REG_HOURS          0x02
#define RV3028_REG_WEEKDAY        0x03
#define RV3028_REG_DATE           0x04
#define RV3028_REG_MONTH          0x05
#define RV3028_REG_YEAR           0x06
#define RV3028_REG_ALARM_MIN      0x07
#define RV3028_REG_ALARM_HOUR     0x08
#define RV3028_REG_ALARM_DATE     0x09
#define RV3028_REG_TIMER_VAL_0    0x0A
#define RV3028_REG_TIMER_VAL_1    0x0B
#define RV3028_REG_TIMER_STAT_0   0x0C
#define RV3028_REG_TIMER_STAT_1   0x0D
#define RV3028_REG_STATUS         0x0E
#define RV3028_REG_CTRL1          0x0F
#define RV3028_REG_CTRL2          0x10
#define RV3028_REG_GP_BITS        0x11
#define RV3028_REG_CLKOUT         0x13
#define RV3028_REG_OFFSET         0x2C
#define RV3028_REG_BACKUP         0x37
#define RV3028_REG_EEPROM_ADDR    0x3D
#define RV3028_REG_EEPROM_DATA    0x3E
#define RV3028_REG_EEPROM_CMD     0x3F
#define RV3028_REG_UNIX_TIME_0    0x1B
#define RV3028_REG_UNIX_TIME_1    0x1C
#define RV3028_REG_UNIX_TIME_2    0x1D
#define RV3028_REG_UNIX_TIME_3    0x1E

#if POCKETOS_RV3028_ENABLE_REGISTER_ACCESS
// Complete register map
static const RegisterDesc RV3028_REGISTERS[] = {
    RegisterDesc(0x00, "SECONDS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "MINUTES", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "HOURS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "WEEKDAY", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x04, "DATE", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x05, "MONTH", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x06, "YEAR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "ALARM_MIN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "ALARM_HOUR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "ALARM_DATE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "TIMER_VAL_0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "TIMER_VAL_1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "TIMER_STAT_0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "TIMER_STAT_1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "STATUS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0F, "CTRL1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x10, "CTRL2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "GP_BITS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "RESERVED_12", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x13, "CLKOUT", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x14, "RESERVED_14", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x15, "RESERVED_15", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x16, "RESERVED_16", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x17, "RESERVED_17", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x18, "RESERVED_18", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x19, "RESERVED_19", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1A, "RESERVED_1A", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1B, "UNIX_TIME_0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1C, "UNIX_TIME_1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1D, "UNIX_TIME_2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1E, "UNIX_TIME_3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1F, "RESERVED_1F", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x20, "RESERVED_20", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x21, "RESERVED_21", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x22, "RESERVED_22", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x23, "RESERVED_23", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x24, "RESERVED_24", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x25, "RESERVED_25", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x26, "RESERVED_26", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x27, "RESERVED_27", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x28, "RESERVED_28", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "RESERVED_29", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "RESERVED_2A", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "RESERVED_2B", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "OFFSET", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2D, "RESERVED_2D", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2E, "RESERVED_2E", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2F, "RESERVED_2F", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x30, "RESERVED_30", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x31, "RESERVED_31", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x32, "RESERVED_32", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x33, "RESERVED_33", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x34, "RESERVED_34", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x35, "RESERVED_35", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x36, "RESERVED_36", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x37, "BACKUP", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x38, "RESERVED_38", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x39, "RESERVED_39", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x3A, "RESERVED_3A", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x3B, "RESERVED_3B", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x3C, "RESERVED_3C", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x3D, "EEPROM_ADDR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3E, "EEPROM_DATA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3F, "EEPROM_CMD", 1, RegisterAccess::RW, 0x00),
};

#define RV3028_REGISTER_COUNT (sizeof(RV3028_REGISTERS) / sizeof(RegisterDesc))
#endif

RV3028Driver::RV3028Driver() : address(0), initialized(false) {
}

bool RV3028Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_RV3028_ENABLE_LOGGING
    Logger::info("RV3028: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t status = 0;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
#if POCKETOS_RV3028_ENABLE_LOGGING
        Logger::error("RV3028: Failed to communicate with device");
#endif
        return false;
    }
    
#if POCKETOS_RV3028_ENABLE_CONFIGURATION
    // Clear power-on reset flag
    if (status & 0x01) {
        status &= ~0x01;
        writeRegister(RV3028_REG_STATUS, status);
    }
#endif
    
    initialized = true;
#if POCKETOS_RV3028_ENABLE_LOGGING
    Logger::info("RV3028: Initialized successfully");
#endif
    return true;
}

void RV3028Driver::deinit() {
    initialized = false;
#if POCKETOS_RV3028_ENABLE_LOGGING
    Logger::info("RV3028: Deinitialized");
#endif
}

RV3028DateTime RV3028Driver::readDateTime() {
    RV3028DateTime dt;
    
    if (!initialized) {
        return dt;
    }
    
    uint8_t buffer[7];
    if (!readRegisters(RV3028_REG_SECONDS, buffer, 7)) {
        return dt;
    }
    
    dt.second = bcdToDec(buffer[0] & 0x7F);
    dt.minute = bcdToDec(buffer[1] & 0x7F);
    dt.hour = bcdToDec(buffer[2] & 0x3F);
    dt.dayOfWeek = buffer[3] & 0x07;
    dt.day = bcdToDec(buffer[4] & 0x3F);
    dt.month = bcdToDec(buffer[5] & 0x1F);
    dt.year = 2000 + bcdToDec(buffer[6]);
    dt.valid = true;
    
    return dt;
}

bool RV3028Driver::setDateTime(const RV3028DateTime& dt) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[7];
    buffer[0] = decToBcd(dt.second);
    buffer[1] = decToBcd(dt.minute);
    buffer[2] = decToBcd(dt.hour);
    buffer[3] = dt.dayOfWeek & 0x07;
    buffer[4] = decToBcd(dt.day);
    buffer[5] = decToBcd(dt.month);
    buffer[6] = decToBcd(dt.year >= 2000 ? dt.year - 2000 : dt.year);
    
    return writeRegisters(RV3028_REG_SECONDS, buffer, 7);
}

uint32_t RV3028Driver::readUnixTime() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t buffer[4];
    if (!readRegisters(RV3028_REG_UNIX_TIME_0, buffer, 4)) {
        return 0;
    }
    
    return ((uint32_t)buffer[3] << 24) | ((uint32_t)buffer[2] << 16) | 
           ((uint32_t)buffer[1] << 8) | buffer[0];
}

bool RV3028Driver::setUnixTime(uint32_t unixTime) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[4];
    buffer[0] = unixTime & 0xFF;
    buffer[1] = (unixTime >> 8) & 0xFF;
    buffer[2] = (unixTime >> 16) & 0xFF;
    buffer[3] = (unixTime >> 24) & 0xFF;
    
    return writeRegisters(RV3028_REG_UNIX_TIME_0, buffer, 4);
}

#if POCKETOS_RV3028_ENABLE_ALARM_FEATURES
bool RV3028Driver::setAlarm(const RV3028Alarm& alarm) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[3];
    buffer[0] = decToBcd(alarm.minute) | (alarm.minuteEnable ? 0x00 : 0x80);
    buffer[1] = decToBcd(alarm.hour) | (alarm.hourEnable ? 0x00 : 0x80);
    buffer[2] = decToBcd(alarm.day) | (alarm.dayEnable ? 0x00 : 0x80);
    
    if (!writeRegisters(RV3028_REG_ALARM_MIN, buffer, 3)) {
        return false;
    }
    
    // Enable/disable alarm interrupt
    uint8_t ctrl2;
    if (!readRegister(RV3028_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    if (alarm.enabled) {
        ctrl2 |= 0x08;  // AIE
    } else {
        ctrl2 &= ~0x08;
    }
    
    return writeRegister(RV3028_REG_CTRL2, ctrl2);
}

RV3028Alarm RV3028Driver::getAlarm() {
    RV3028Alarm alarm;
    if (!initialized) {
        return alarm;
    }
    
    uint8_t buffer[3];
    if (readRegisters(RV3028_REG_ALARM_MIN, buffer, 3)) {
        alarm.minute = bcdToDec(buffer[0] & 0x7F);
        alarm.minuteEnable = (buffer[0] & 0x80) == 0;
        alarm.hour = bcdToDec(buffer[1] & 0x3F);
        alarm.hourEnable = (buffer[1] & 0x80) == 0;
        alarm.day = bcdToDec(buffer[2] & 0x3F);
        alarm.dayEnable = (buffer[2] & 0x80) == 0;
        
        uint8_t ctrl2;
        if (readRegister(RV3028_REG_CTRL2, &ctrl2)) {
            alarm.enabled = (ctrl2 & 0x08) != 0;
        }
    }
    
    return alarm;
}

bool RV3028Driver::checkAlarmFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
        return false;
    }
    
    return (status & 0x04) != 0;
}

bool RV3028Driver::clearAlarmFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
        return false;
    }
    
    status &= ~0x04;
    return writeRegister(RV3028_REG_STATUS, status);
}

bool RV3028Driver::setCountdownTimer(uint16_t ticks, uint8_t source, bool repeat) {
    if (!initialized || source > 3) {
        return false;
    }
    
    uint8_t buffer[2];
    buffer[0] = ticks & 0xFF;
    buffer[1] = (ticks >> 8) & 0xFF;
    
    if (!writeRegisters(RV3028_REG_TIMER_VAL_0, buffer, 2)) {
        return false;
    }
    
    uint8_t ctrl = (source & 0x03);
    if (repeat) {
        ctrl |= 0x08;
    }
    
    return writeRegister(RV3028_REG_TIMER_STAT_0, ctrl);
}

bool RV3028Driver::enableTimer(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(RV3028_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    if (enable) {
        ctrl2 |= 0x04;  // TIE
        ctrl2 |= 0x01;  // TE (timer enable)
    } else {
        ctrl2 &= ~0x04;
        ctrl2 &= ~0x01;
    }
    
    return writeRegister(RV3028_REG_CTRL2, ctrl2);
}

bool RV3028Driver::checkTimerFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
        return false;
    }
    
    return (status & 0x08) != 0;
}

bool RV3028Driver::clearTimerFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
        return false;
    }
    
    status &= ~0x08;
    return writeRegister(RV3028_REG_STATUS, status);
}

bool RV3028Driver::setPeriodicTimeUpdate(uint8_t period) {
    if (!initialized || period > 6) {
        return false;
    }
    
    uint8_t ctrl1;
    if (!readRegister(RV3028_REG_CTRL1, &ctrl1)) {
        return false;
    }
    
    ctrl1 = (ctrl1 & ~0x70) | ((period & 0x07) << 4);
    return writeRegister(RV3028_REG_CTRL1, ctrl1);
}

bool RV3028Driver::enablePeriodicUpdate(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(RV3028_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    if (enable) {
        ctrl2 |= 0x20;  // UIE
    } else {
        ctrl2 &= ~0x20;
    }
    
    return writeRegister(RV3028_REG_CTRL2, ctrl2);
}

bool RV3028Driver::checkUpdateFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
        return false;
    }
    
    return (status & 0x10) != 0;
}

bool RV3028Driver::clearUpdateFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
        return false;
    }
    
    status &= ~0x10;
    return writeRegister(RV3028_REG_STATUS, status);
}

bool RV3028Driver::setClockOutput(bool enable, uint8_t freq) {
    if (!initialized || freq > 5) {
        return false;
    }
    
    uint8_t clkout = freq & 0x07;
    if (!enable) {
        clkout = 0;
    }
    
    return writeRegister(RV3028_REG_CLKOUT, clkout);
}

bool RV3028Driver::readEEPROM(uint8_t address, uint8_t* data, uint8_t length) {
    if (!initialized || address >= 43 || (address + length) > 43) {
        return false;
    }
    
    for (uint8_t i = 0; i < length; i++) {
        // Set EEPROM address
        if (!writeRegister(RV3028_REG_EEPROM_ADDR, address + i)) {
            return false;
        }
        
        // Issue read command (0x22)
        if (!writeRegister(RV3028_REG_EEPROM_CMD, 0x22)) {
            return false;
        }
        
        // Wait for completion
        delay(5);
        
        // Read data
        if (!readRegister(RV3028_REG_EEPROM_DATA, &data[i])) {
            return false;
        }
    }
    
    return true;
}

bool RV3028Driver::writeEEPROM(uint8_t address, const uint8_t* data, uint8_t length) {
    if (!initialized || address >= 43 || (address + length) > 43) {
        return false;
    }
    
    for (uint8_t i = 0; i < length; i++) {
        // Set EEPROM address
        if (!writeRegister(RV3028_REG_EEPROM_ADDR, address + i)) {
            return false;
        }
        
        // Write data
        if (!writeRegister(RV3028_REG_EEPROM_DATA, data[i])) {
            return false;
        }
        
        // Issue write command (0x21)
        if (!writeRegister(RV3028_REG_EEPROM_CMD, 0x21)) {
            return false;
        }
        
        // Wait for completion
        delay(5);
    }
    
    return true;
}

bool RV3028Driver::setTrickleCharger(uint8_t resistor, uint8_t diode) {
    if (!initialized || resistor > 4) {
        return false;
    }
    
    uint8_t backup = 0;
    if (resistor > 0) {
        backup = 0x20 | ((resistor & 0x07) << 2) | (diode & 0x03);
    }
    
    return writeRegister(RV3028_REG_BACKUP, backup);
}

bool RV3028Driver::setOffset(int8_t offset) {
    if (!initialized) {
        return false;
    }
    
    return writeRegister(RV3028_REG_OFFSET, (uint8_t)offset);
}

int8_t RV3028Driver::getOffset() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t offset;
    if (!readRegister(RV3028_REG_OFFSET, &offset)) {
        return 0;
    }
    
    return (int8_t)offset;
}

bool RV3028Driver::enableBackupSwitchover(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint8_t backup;
    if (!readRegister(RV3028_REG_BACKUP, &backup)) {
        return false;
    }
    
    if (enable) {
        backup |= 0x80;  // BSM enable
    } else {
        backup &= ~0x80;
    }
    
    return writeRegister(RV3028_REG_BACKUP, backup);
}

bool RV3028Driver::checkBatteryLow() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(RV3028_REG_STATUS, &status)) {
        return false;
    }
    
    return (status & 0x02) != 0;
}
#endif

CapabilitySchema RV3028Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "rv3028";
    schema.tier = POCKETOS_RV3028_TIER_NAME;
    schema.description = "RV3028 Ultra-Low Power RTC";
    schema.capabilities = "datetime_read,datetime_write,unix_time";
    
#if POCKETOS_RV3028_ENABLE_ALARM_FEATURES
    schema.capabilities += ",alarm,countdown_timer,periodic_update,clock_output,eeprom,trickle_charger,offset_calibration,battery_switchover";
#endif
    
    return schema;
}

String RV3028Driver::getParameter(const String& name) {
    if (name == "time") {
        RV3028DateTime dt = readDateTime();
        if (dt.valid) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            return String(buf);
        }
    } else if (name == "unix_time") {
        return String(readUnixTime());
    }
    return "";
}

bool RV3028Driver::setParameter(const String& name, const String& value) {
    if (name == "time" && value.length() >= 19) {
        RV3028DateTime dt;
        dt.year = value.substring(0, 4).toInt();
        dt.month = value.substring(5, 7).toInt();
        dt.day = value.substring(8, 10).toInt();
        dt.hour = value.substring(11, 13).toInt();
        dt.minute = value.substring(14, 16).toInt();
        dt.second = value.substring(17, 19).toInt();
        dt.dayOfWeek = 0;
        return setDateTime(dt);
    } else if (name == "unix_time") {
        return setUnixTime(value.toInt());
    }
    return false;
}

#if POCKETOS_RV3028_ENABLE_REGISTER_ACCESS
const RegisterDesc* RV3028Driver::registers(size_t& count) const {
    count = RV3028_REGISTER_COUNT;
    return RV3028_REGISTERS;
}

bool RV3028Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x3F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        RV3028_REGISTERS, RV3028_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool RV3028Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x3F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        RV3028_REGISTERS, RV3028_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* RV3028Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(RV3028_REGISTERS, RV3028_REGISTER_COUNT, name);
}
#endif

bool RV3028Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() != 1) {
        return false;
    }
    
    *value = Wire.read();
    return true;
}

bool RV3028Driver::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, length);
    if (Wire.available() != length) {
        return false;
    }
    
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}

bool RV3028Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool RV3028Driver::writeRegisters(uint8_t reg, const uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(buffer[i]);
    }
    return Wire.endTransmission() == 0;
}

uint8_t RV3028Driver::bcdToDec(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

uint8_t RV3028Driver::decToBcd(uint8_t val) {
    return ((val / 10) * 16) + (val % 10);
}

} // namespace PocketOS
