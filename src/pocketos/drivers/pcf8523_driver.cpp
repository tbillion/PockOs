#include "pcf8523_driver.h"
#include "../driver_config.h"

#if POCKETOS_PCF8523_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// PCF8523 Register addresses
#define PCF8523_REG_CTRL1         0x00
#define PCF8523_REG_CTRL2         0x01
#define PCF8523_REG_CTRL3         0x02
#define PCF8523_REG_SECONDS       0x03
#define PCF8523_REG_MINUTES       0x04
#define PCF8523_REG_HOURS         0x05
#define PCF8523_REG_DAYS          0x06
#define PCF8523_REG_WEEKDAYS      0x07
#define PCF8523_REG_MONTHS        0x08
#define PCF8523_REG_YEARS         0x09
#define PCF8523_REG_ALARM_MIN     0x0A
#define PCF8523_REG_ALARM_HOUR    0x0B
#define PCF8523_REG_ALARM_DAY     0x0C
#define PCF8523_REG_ALARM_WDAY    0x0D
#define PCF8523_REG_OFFSET        0x0E
#define PCF8523_REG_TMR_CLKOUT    0x0F
#define PCF8523_REG_TMR_A_FREQ    0x10
#define PCF8523_REG_TMR_A_REG     0x11
#define PCF8523_REG_TMR_B_FREQ    0x12
#define PCF8523_REG_TMR_B_REG     0x13

#if POCKETOS_PCF8523_ENABLE_REGISTER_ACCESS
// Complete register map
static const RegisterDesc PCF8523_REGISTERS[] = {
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
    RegisterDesc(0x0A, "ALARM_MIN", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0B, "ALARM_HOUR", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0C, "ALARM_DAY", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0D, "ALARM_WDAY", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x0E, "OFFSET", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0F, "TMR_CLKOUT", 1, RegisterAccess::RW, 0x38),
    RegisterDesc(0x10, "TMR_A_FREQ", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "TMR_A_REG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "TMR_B_FREQ", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "TMR_B_REG", 1, RegisterAccess::RW, 0x00),
};

#define PCF8523_REGISTER_COUNT (sizeof(PCF8523_REGISTERS) / sizeof(RegisterDesc))
#endif

PCF8523Driver::PCF8523Driver() : address(0), initialized(false) {
}

bool PCF8523Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_PCF8523_ENABLE_LOGGING
    Logger::info("PCF8523: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t ctrl1 = 0;
    if (!readRegister(PCF8523_REG_CTRL1, &ctrl1)) {
#if POCKETOS_PCF8523_ENABLE_LOGGING
        Logger::error("PCF8523: Failed to communicate with device");
#endif
        return false;
    }
    
#if POCKETOS_PCF8523_ENABLE_CONFIGURATION
    // Clear CAP_SEL and enable 12.5pF capacitor by default
    ctrl1 &= ~0x80;
    if (!writeRegister(PCF8523_REG_CTRL1, ctrl1)) {
#if POCKETOS_PCF8523_ENABLE_LOGGING
        Logger::error("PCF8523: Failed to configure device");
#endif
        return false;
    }
    
    // Set battery switchover to standard mode
    uint8_t ctrl3 = 0x00;
    writeRegister(PCF8523_REG_CTRL3, ctrl3);
#endif
    
    initialized = true;
#if POCKETOS_PCF8523_ENABLE_LOGGING
    Logger::info("PCF8523: Initialized successfully");
#endif
    return true;
}

void PCF8523Driver::deinit() {
    initialized = false;
#if POCKETOS_PCF8523_ENABLE_LOGGING
    Logger::info("PCF8523: Deinitialized");
#endif
}

PCF8523DateTime PCF8523Driver::readDateTime() {
    PCF8523DateTime dt;
    
    if (!initialized) {
        return dt;
    }
    
    uint8_t buffer[7];
    if (!readRegisters(PCF8523_REG_SECONDS, buffer, 7)) {
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

bool PCF8523Driver::setDateTime(const PCF8523DateTime& dt) {
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
    
    return writeRegisters(PCF8523_REG_SECONDS, buffer, 7);
}

#if POCKETOS_PCF8523_ENABLE_ALARM_FEATURES
bool PCF8523Driver::setAlarm(const PCF8523Alarm& alarm) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[3];
    buffer[0] = decToBcd(alarm.minute) | (alarm.minuteEnable ? 0x00 : 0x80);
    buffer[1] = decToBcd(alarm.hour) | (alarm.hourEnable ? 0x00 : 0x80);
    buffer[2] = decToBcd(alarm.day) | (alarm.dayEnable ? 0x00 : 0x80);
    
    if (!writeRegisters(PCF8523_REG_ALARM_MIN, buffer, 3)) {
        return false;
    }
    
    // Enable/disable alarm interrupt
    uint8_t ctrl2;
    if (!readRegister(PCF8523_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    if (alarm.enabled) {
        ctrl2 |= 0x02;  // AIE
    } else {
        ctrl2 &= ~0x02;
    }
    
    return writeRegister(PCF8523_REG_CTRL2, ctrl2);
}

PCF8523Alarm PCF8523Driver::getAlarm() {
    PCF8523Alarm alarm;
    if (!initialized) {
        return alarm;
    }
    
    uint8_t buffer[3];
    if (readRegisters(PCF8523_REG_ALARM_MIN, buffer, 3)) {
        alarm.minute = bcdToDec(buffer[0] & 0x7F);
        alarm.minuteEnable = (buffer[0] & 0x80) == 0;
        alarm.hour = bcdToDec(buffer[1] & 0x3F);
        alarm.hourEnable = (buffer[1] & 0x80) == 0;
        alarm.day = bcdToDec(buffer[2] & 0x3F);
        alarm.dayEnable = (buffer[2] & 0x80) == 0;
        
        uint8_t ctrl2;
        if (readRegister(PCF8523_REG_CTRL2, &ctrl2)) {
            alarm.enabled = (ctrl2 & 0x02) != 0;
        }
    }
    
    return alarm;
}

bool PCF8523Driver::checkAlarmFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF8523_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    return (ctrl2 & 0x08) != 0;
}

bool PCF8523Driver::clearAlarmFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF8523_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    ctrl2 &= ~0x08;
    return writeRegister(PCF8523_REG_CTRL2, ctrl2);
}

bool PCF8523Driver::setCountdownTimer(uint8_t source, uint8_t value, bool repeat) {
    if (!initialized || source > 3) {
        return false;
    }
    
    uint8_t freq = (source & 0x03);
    if (repeat) {
        freq |= 0x08;
    }
    
    return writeRegister(PCF8523_REG_TMR_A_FREQ, freq) &&
           writeRegister(PCF8523_REG_TMR_A_REG, value);
}

bool PCF8523Driver::enableTimer(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF8523_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    if (enable) {
        ctrl2 |= 0x01;  // CTAIE
    } else {
        ctrl2 &= ~0x01;
    }
    
    return writeRegister(PCF8523_REG_CTRL2, ctrl2);
}

bool PCF8523Driver::checkTimerFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF8523_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    return (ctrl2 & 0x04) != 0;
}

bool PCF8523Driver::clearTimerFlag() {
    if (!initialized) {
        return false;
    }
    
    uint8_t ctrl2;
    if (!readRegister(PCF8523_REG_CTRL2, &ctrl2)) {
        return false;
    }
    
    ctrl2 &= ~0x04;
    return writeRegister(PCF8523_REG_CTRL2, ctrl2);
}

bool PCF8523Driver::setClockOutput(bool enable, uint8_t freq) {
    if (!initialized || freq > 7) {
        return false;
    }
    
    uint8_t clkout;
    if (!readRegister(PCF8523_REG_TMR_CLKOUT, &clkout)) {
        return false;
    }
    
    clkout = (clkout & ~0x38) | ((freq & 0x07) << 3);
    if (!enable) {
        clkout |= 0x80;
    } else {
        clkout &= ~0x80;
    }
    
    return writeRegister(PCF8523_REG_TMR_CLKOUT, clkout);
}

bool PCF8523Driver::setOffset(uint8_t mode, uint8_t offset) {
    if (!initialized || offset > 63) {
        return false;
    }
    
    uint8_t val = offset & 0x3F;
    if (mode == 1) {
        val |= 0x80;  // Fast mode
    }
    
    return writeRegister(PCF8523_REG_OFFSET, val);
}

bool PCF8523Driver::setBatteryMode(uint8_t mode) {
    if (!initialized || mode > 2) {
        return false;
    }
    
    uint8_t ctrl3;
    if (!readRegister(PCF8523_REG_CTRL3, &ctrl3)) {
        return false;
    }
    
    ctrl3 = (ctrl3 & ~0xE0);
    
    switch (mode) {
        case 0:  // Standard
            ctrl3 |= 0x00;
            break;
        case 1:  // Direct switching
            ctrl3 |= 0xE0;
            break;
        case 2:  // Low power
            ctrl3 |= 0x60;
            break;
    }
    
    return writeRegister(PCF8523_REG_CTRL3, ctrl3);
}

bool PCF8523Driver::setCapacitorSelection(uint8_t cap) {
    if (!initialized || cap > 1) {
        return false;
    }
    
    uint8_t ctrl1;
    if (!readRegister(PCF8523_REG_CTRL1, &ctrl1)) {
        return false;
    }
    
    if (cap == 0) {
        ctrl1 |= 0x80;  // 7pF
    } else {
        ctrl1 &= ~0x80;  // 12.5pF
    }
    
    return writeRegister(PCF8523_REG_CTRL1, ctrl1);
}
#endif

CapabilitySchema PCF8523Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "pcf8523";
    schema.tier = POCKETOS_PCF8523_TIER_NAME;
    schema.description = "PCF8523 Low Power RTC";
    schema.capabilities = "datetime_read,datetime_write";
    
#if POCKETOS_PCF8523_ENABLE_ALARM_FEATURES
    schema.capabilities += ",alarm,countdown_timer,clock_output,offset_calibration,battery_mode";
#endif
    
    return schema;
}

String PCF8523Driver::getParameter(const String& name) {
    if (name == "time") {
        PCF8523DateTime dt = readDateTime();
        if (dt.valid) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            return String(buf);
        }
    }
    return "";
}

bool PCF8523Driver::setParameter(const String& name, const String& value) {
    if (name == "time" && value.length() >= 19) {
        PCF8523DateTime dt;
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

#if POCKETOS_PCF8523_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCF8523Driver::registers(size_t& count) const {
    count = PCF8523_REGISTER_COUNT;
    return PCF8523_REGISTERS;
}

bool PCF8523Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x13 || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        PCF8523_REGISTERS, PCF8523_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool PCF8523Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x13 || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        PCF8523_REGISTERS, PCF8523_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* PCF8523Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCF8523_REGISTERS, PCF8523_REGISTER_COUNT, name);
}
#endif

bool PCF8523Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool PCF8523Driver::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length) {
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

bool PCF8523Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool PCF8523Driver::writeRegisters(uint8_t reg, const uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(buffer[i]);
    }
    return Wire.endTransmission() == 0;
}

uint8_t PCF8523Driver::bcdToDec(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

uint8_t PCF8523Driver::decToBcd(uint8_t val) {
    return ((val / 10) * 16) + (val % 10);
}

} // namespace PocketOS
