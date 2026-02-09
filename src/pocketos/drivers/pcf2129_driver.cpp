#include "pcf2129_driver.h"
#include "../driver_config.h"

#if POCKETOS_PCF2129_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// PCF2129 Register addresses
#define PCF2129_REG_CTRL1         0x00
#define PCF2129_REG_CTRL2         0x01
#define PCF2129_REG_CTRL3         0x02
#define PCF2129_REG_SECONDS       0x03
#define PCF2129_REG_MINUTES       0x04
#define PCF2129_REG_HOURS         0x05
#define PCF2129_REG_DAYS          0x06
#define PCF2129_REG_WEEKDAYS      0x07
#define PCF2129_REG_MONTHS        0x08
#define PCF2129_REG_YEARS         0x09
#define PCF2129_REG_ALARM_SEC     0x0A
#define PCF2129_REG_ALARM_MIN     0x0B
#define PCF2129_REG_ALARM_HOUR    0x0C
#define PCF2129_REG_ALARM_DAY     0x0D
#define PCF2129_REG_ALARM_WDAY    0x0E
#define PCF2129_REG_CLKOUT_CTL    0x0F
#define PCF2129_REG_TIMESTP_CTL   0x10
#define PCF2129_REG_SEC_TIMESTP   0x11
#define PCF2129_REG_MIN_TIMESTP   0x12
#define PCF2129_REG_HOUR_TIMESTP  0x13
#define PCF2129_REG_DAY_TIMESTP   0x14
#define PCF2129_REG_MON_TIMESTP   0x15
#define PCF2129_REG_YEAR_TIMESTP  0x16
#define PCF2129_REG_AGING_OFFSET  0x19
#define PCF2129_REG_INT_A_MASK1   0x1A
#define PCF2129_REG_INT_A_MASK2   0x1B
#define PCF2129_REG_INT_B_MASK1   0x1C
#define PCF2129_REG_INT_B_MASK2   0x1D
#define PCF2129_REG_WATCHDG_TIM_CTL 0x1E
#define PCF2129_REG_WATCHDG_TIM_VAL 0x1F

#if POCKETOS_PCF2129_ENABLE_REGISTER_ACCESS
// Complete register map
static const RegisterDesc PCF2129_REGISTERS[] = {
    RegisterDesc(0x00, "CTRL1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "CTRL2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "CTRL3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "SECONDS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x04, "MINUTES", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "HOURS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "DAYS", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x07, "WEEKDAYS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "MONTHS", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x09, "YEARS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "ALARM_SEC", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0B, "ALARM_MIN", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0C, "ALARM_HOUR", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0D, "ALARM_DAY", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0E, "ALARM_WDAY", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0F, "CLKOUT_CTL", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x10, "TIMESTP_CTL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "SEC_TIMESTP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x12, "MIN_TIMESTP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x13, "HOUR_TIMESTP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x14, "DAY_TIMESTP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x15, "MON_TIMESTP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x16, "YEAR_TIMESTP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x17, "RESERVED_17", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x18, "RESERVED_18", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x19, "AGING_OFFSET", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1A, "INT_A_MASK1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1B, "INT_A_MASK2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1C, "INT_B_MASK1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1D, "INT_B_MASK2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1E, "WATCHDG_TIM_CTL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1F, "WATCHDG_TIM_VAL", 1, RegisterAccess::RW, 0x00),
};

#define PCF2129_REGISTER_COUNT (sizeof(PCF2129_REGISTERS) / sizeof(RegisterDesc))
#endif

PCF2129Driver::PCF2129Driver() : address(0), initialized(false) {
}

bool PCF2129Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_PCF2129_ENABLE_LOGGING
    Logger::info("PCF2129: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t ctrl1 = 0;
    if (!readRegister(PCF2129_REG_CTRL1, &ctrl1)) {
#if POCKETOS_PCF2129_ENABLE_LOGGING
        Logger::error("PCF2129: Failed to communicate with device");
#endif
        return false;
    }
    
#if POCKETOS_PCF2129_ENABLE_CONFIGURATION
    // Clear software reset and stop bits
    ctrl1 &= ~0x58;
    if (!writeRegister(PCF2129_REG_CTRL1, ctrl1)) {
#if POCKETOS_PCF2129_ENABLE_LOGGING
        Logger::error("PCF2129: Failed to configure device");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_PCF2129_ENABLE_LOGGING
    Logger::info("PCF2129: Initialized successfully");
#endif
    return true;
}

void PCF2129Driver::deinit() {
    initialized = false;
#if POCKETOS_PCF2129_ENABLE_LOGGING
    Logger::info("PCF2129: Deinitialized");
#endif
}

PCF2129DateTime PCF2129Driver::readDateTime() {
    PCF2129DateTime dt;
    
    if (!initialized) {
        return dt;
    }
    
    uint8_t buffer[7];
    if (!readRegisters(PCF2129_REG_SECONDS, buffer, 7)) {
        return dt;
    }
    
    dt.second = bcdToDec(buffer[0] & 0x7F);
    dt.minute = bcdToDec(buffer[1] & 0x7F);
    dt.hour = bcdToDec(buffer[2] & 0x3F);
    dt.day = bcdToDec(buffer[3] & 0x3F);
    dt.dayOfWeek = buffer[4] & 0x07;
    dt.month = bcdToDec(buffer[5] & 0x1F);
    dt.year = 2000 + bcdToDec(buffer[6]);
    dt.valid = true;
    
    return dt;
}

bool PCF2129Driver::setDateTime(const PCF2129DateTime& dt) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[7];
    buffer[0] = decToBcd(dt.second);
    buffer[1] = decToBcd(dt.minute);
    buffer[2] = decToBcd(dt.hour);
    buffer[3] = decToBcd(dt.day);
    buffer[4] = dt.dayOfWeek & 0x07;
    buffer[5] = decToBcd(dt.month);
    buffer[6] = decToBcd(dt.year >= 2000 ? dt.year - 2000 : dt.year);
    
    return writeRegisters(PCF2129_REG_SECONDS, buffer, 7);
}

#if POCKETOS_PCF2129_ENABLE_ALARM_FEATURES
bool PCF2129Driver::setAlarm(const PCF2129Alarm& alarm) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[4];
    buffer[0] = decToBcd(alarm.second) | (alarm.secondEnable ? 0x00 : 0x80);
    buffer[1] = decToBcd(alarm.minute) | (alarm.minuteEnable ? 0x00 : 0x80);
    buffer[2] = decToBcd(alarm.hour) | (alarm.hourEnable ? 0x00 : 0x80);
    buffer[3] = decToBcd(alarm.day) | (alarm.dayEnable ? 0x00 : 0x80);
    
    if (!writeRegisters(PCF2129_REG_ALARM_SEC, buffer, 4)) {
        return false;
    }
    
    // Enable/disable alarm interrupt
    uint8_t ctrl2;
    if (!readRegister(PCF2129_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    if (alarm.enabled) {
        ctrl2 |= 0x02;  // AIE
    } else {
        ctrl2 &= ~0x02;
    }
    
    return writeRegister(PCF2129_REG_CTRL2, ctrl2);
}

PCF2129Alarm PCF2129Driver::getAlarm() {
    PCF2129Alarm alarm;
    if (!initialized) {
        return alarm;
    }
    
    uint8_t buffer[4];
    if (readRegisters(PCF2129_REG_ALARM_SEC, buffer, 4)) {
        alarm.second = bcdToDec(buffer[0] & 0x7F);
        alarm.secondEnable = (buffer[0] & 0x80) == 0;
        alarm.minute = bcdToDec(buffer[1] & 0x7F);
        alarm.minuteEnable = (buffer[1] & 0x80) == 0;
        alarm.hour = bcdToDec(buffer[2] & 0x3F);
        alarm.hourEnable = (buffer[2] & 0x80) == 0;
        alarm.day = bcdToDec(buffer[3] & 0x3F);
        alarm.dayEnable = (buffer[3] & 0x80) == 0;
        
        uint8_t ctrl2;
        if (readRegister(PCF2129_REG_CTRL2, &ctrl2)) {
            alarm.enabled = (ctrl2 & 0x02) != 0;
        }
    }
    
    return alarm;
}

bool PCF2129Driver::checkAlarmFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF2129_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    return (ctrl2 & 0x40) != 0;
}

bool PCF2129Driver::clearAlarmFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF2129_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    ctrl2 &= ~0x40;
    return writeRegister(PCF2129_REG_CTRL2, ctrl2);
}

bool PCF2129Driver::setTimer(uint16_t seconds, bool repeat) {
    if (!initialized) {
        return false;
    }
    
    // PCF2129 timer implementation
    uint8_t watchdog = (seconds & 0xFF);
    uint8_t ctrl = repeat ? 0x02 : 0x00;
    
    return writeRegister(PCF2129_REG_WATCHDG_TIM_VAL, watchdog) &&
           writeRegister(PCF2129_REG_WATCHDG_TIM_CTL, ctrl);
}

bool PCF2129Driver::enableTimer(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF2129_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    if (enable) {
        ctrl2 |= 0x01;  // TIE
    } else {
        ctrl2 &= ~0x01;
    }
    
    return writeRegister(PCF2129_REG_CTRL2, ctrl2);
}

bool PCF2129Driver::checkTimerFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF2129_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    return (ctrl2 & 0x20) != 0;
}

bool PCF2129Driver::clearTimerFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF2129_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    ctrl2 &= ~0x20;
    return writeRegister(PCF2129_REG_CTRL2, ctrl2);
}

bool PCF2129Driver::setClockOutput(bool enable, uint8_t freq) {
    if (!initialized || freq > 7) {
        return false;
    }
    
    uint8_t clkout = freq & 0x07;
    if (!enable) {
        clkout |= 0x80;
    }
    
    return writeRegister(PCF2129_REG_CLKOUT_CTL, clkout);
}

bool PCF2129Driver::enableTimestamp(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl = enable ? 0x80 : 0x00;
    return writeRegister(PCF2129_REG_TIMESTP_CTL, ctrl);
}

bool PCF2129Driver::getTimestamp(PCF2129DateTime& dt) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[6];
    if (!readRegisters(PCF2129_REG_SEC_TIMESTP, buffer, 6)) {
        return false;
    }
    
    dt.second = bcdToDec(buffer[0] & 0x7F);
    dt.minute = bcdToDec(buffer[1] & 0x7F);
    dt.hour = bcdToDec(buffer[2] & 0x3F);
    dt.day = bcdToDec(buffer[3] & 0x3F);
    dt.month = bcdToDec(buffer[4] & 0x1F);
    dt.year = 2000 + bcdToDec(buffer[5]);
    dt.valid = true;
    
    return true;
}

bool PCF2129Driver::clearTimestampFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl;
    if (!readRegister(PCF2129_REG_TIMESTP_CTL, &ctrl)) {
        return false;
    }
    
    ctrl &= ~0x40;
    return writeRegister(PCF2129_REG_TIMESTP_CTL, ctrl);
}

bool PCF2129Driver::setAgingOffset(uint8_t offset) {
    if (!initialized || offset > 3) {
        return false;
    }
    
    return writeRegister(PCF2129_REG_AGING_OFFSET, offset);
}

uint8_t PCF2129Driver::getAgingOffset() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t offset;
    if (!readRegister(PCF2129_REG_AGING_OFFSET, &offset)) {
        return 0;
    }
    
    return offset & 0x03;
}
#endif

CapabilitySchema PCF2129Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "pcf2129";
    schema.tier = POCKETOS_PCF2129_TIER_NAME;
    schema.description = "PCF2129 High Accuracy RTC";
    schema.capabilities = "datetime_read,datetime_write";
    
#if POCKETOS_PCF2129_ENABLE_ALARM_FEATURES
    schema.capabilities += ",alarm,timer,timestamp,clock_output,aging_offset";
#endif
    
    return schema;
}

String PCF2129Driver::getParameter(const String& name) {
    if (name == "time") {
        PCF2129DateTime dt = readDateTime();
        if (dt.valid) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            return String(buf);
        }
    }
    return "";
}

bool PCF2129Driver::setParameter(const String& name, const String& value) {
    if (name == "time" && value.length() >= 19) {
        PCF2129DateTime dt;
        dt.year = value.substring(0, 4).toInt();
        dt.month = value.substring(5, 7).toInt();
        dt.day = value.substring(8, 10).toInt();
        dt.hour = value.substring(11, 13).toInt();
        dt.minute = value.substring(14, 16).toInt();
        dt.second = value.substring(17, 19).toInt();
        dt.dayOfWeek = 0;
        return setDateTime(dt);
    }
    return false;
}

#if POCKETOS_PCF2129_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCF2129Driver::registers(size_t& count) const {
    count = PCF2129_REGISTER_COUNT;
    return PCF2129_REGISTERS;
}

bool PCF2129Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x1F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        PCF2129_REGISTERS, PCF2129_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool PCF2129Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x1F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        PCF2129_REGISTERS, PCF2129_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* PCF2129Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCF2129_REGISTERS, PCF2129_REGISTER_COUNT, name);
}
#endif

bool PCF2129Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool PCF2129Driver::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length) {
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

bool PCF2129Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool PCF2129Driver::writeRegisters(uint8_t reg, const uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(buffer[i]);
    }
    return Wire.endTransmission() == 0;
}

uint8_t PCF2129Driver::bcdToDec(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

uint8_t PCF2129Driver::decToBcd(uint8_t val) {
    return ((val / 10) * 16) + (val % 10);
}

} // namespace PocketOS
