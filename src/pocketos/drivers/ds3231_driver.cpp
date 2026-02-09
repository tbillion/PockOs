#include "ds3231_driver.h"
#include "../driver_config.h"

#if POCKETOS_DS3231_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// DS3231 Register addresses
#define DS3231_REG_SECONDS     0x00
#define DS3231_REG_MINUTES     0x01
#define DS3231_REG_HOURS       0x02
#define DS3231_REG_DAY         0x03
#define DS3231_REG_DATE        0x04
#define DS3231_REG_MONTH       0x05
#define DS3231_REG_YEAR        0x06
#define DS3231_REG_ALARM1_SEC  0x07
#define DS3231_REG_ALARM1_MIN  0x08
#define DS3231_REG_ALARM1_HOUR 0x09
#define DS3231_REG_ALARM1_DATE 0x0A
#define DS3231_REG_ALARM2_MIN  0x0B
#define DS3231_REG_ALARM2_HOUR 0x0C
#define DS3231_REG_ALARM2_DATE 0x0D
#define DS3231_REG_CONTROL     0x0E
#define DS3231_REG_STATUS      0x0F
#define DS3231_REG_AGING       0x10
#define DS3231_REG_TEMP_MSB    0x11
#define DS3231_REG_TEMP_LSB    0x12

#if POCKETOS_DS3231_ENABLE_REGISTER_ACCESS
static const RegisterDesc DS3231_REGISTERS[] = {
    RegisterDesc(0x00, "SECONDS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "MINUTES", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "HOURS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "DAY", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x04, "DATE", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x05, "MONTH", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x06, "YEAR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "ALARM1_SEC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "ALARM1_MIN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "ALARM1_HOUR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "ALARM1_DATE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "ALARM2_MIN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "ALARM2_HOUR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "ALARM2_DATE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "CONTROL", 1, RegisterAccess::RW, 0x1C),
    RegisterDesc(0x0F, "STATUS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x10, "AGING_OFFSET", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "TEMP_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x12, "TEMP_LSB", 1, RegisterAccess::RO, 0x00),
};

#define DS3231_REGISTER_COUNT (sizeof(DS3231_REGISTERS) / sizeof(RegisterDesc))
#endif

DS3231Driver::DS3231Driver() : address(0), initialized(false) {
}

bool DS3231Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_DS3231_ENABLE_LOGGING
    Logger::info("DS3231: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t status = 0;
    if (!readRegister(DS3231_REG_STATUS, &status)) {
#if POCKETOS_DS3231_ENABLE_LOGGING
        Logger::error("DS3231: Failed to communicate with device");
#endif
        return false;
    }
    
#if POCKETOS_DS3231_ENABLE_CONFIGURATION
    // Clear OSF flag if set
    if (status & 0x80) {
        status &= ~0x80;
        writeRegister(DS3231_REG_STATUS, status);
    }
#endif
    
    initialized = true;
#if POCKETOS_DS3231_ENABLE_LOGGING
    Logger::info("DS3231: Initialized successfully");
#endif
    return true;
}

void DS3231Driver::deinit() {
    initialized = false;
#if POCKETOS_DS3231_ENABLE_LOGGING
    Logger::info("DS3231: Deinitialized");
#endif
}

DS3231DateTime DS3231Driver::readDateTime() {
    DS3231DateTime dt;
    
    if (!initialized) {
        return dt;
    }
    
    uint8_t buffer[7];
    if (!readRegisters(DS3231_REG_SECONDS, buffer, 7)) {
        return dt;
    }
    
    dt.second = bcdToDec(buffer[0] & 0x7F);
    dt.minute = bcdToDec(buffer[1] & 0x7F);
    dt.hour = bcdToDec(buffer[2] & 0x3F);
    dt.dayOfWeek = bcdToDec(buffer[3] & 0x07);
    dt.day = bcdToDec(buffer[4] & 0x3F);
    dt.month = bcdToDec(buffer[5] & 0x1F);
    dt.year = 2000 + bcdToDec(buffer[6]);
    dt.valid = true;
    
    return dt;
}

bool DS3231Driver::setDateTime(const DS3231DateTime& dt) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[7];
    buffer[0] = decToBcd(dt.second);
    buffer[1] = decToBcd(dt.minute);
    buffer[2] = decToBcd(dt.hour);
    buffer[3] = decToBcd(dt.dayOfWeek);
    buffer[4] = decToBcd(dt.day);
    buffer[5] = decToBcd(dt.month);
    buffer[6] = decToBcd(dt.year >= 2000 ? dt.year - 2000 : dt.year);
    
    return writeRegisters(DS3231_REG_SECONDS, buffer, 7);
}

float DS3231Driver::readTemperature() {
    if (!initialized) {
        return -999.0f;
    }
    
    uint8_t msb, lsb;
    if (!readRegister(DS3231_REG_TEMP_MSB, &msb) || !readRegister(DS3231_REG_TEMP_LSB, &lsb)) {
        return -999.0f;
    }
    
    int16_t temp = ((int16_t)msb << 8) | lsb;
    return temp / 256.0f;
}

#if POCKETOS_DS3231_ENABLE_ALARM_FEATURES
bool DS3231Driver::setAlarm1(const DS3231Alarm& alarm) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[4];
    buffer[0] = decToBcd(alarm.second) | (alarm.mode & 0x80);
    buffer[1] = decToBcd(alarm.minute) | (alarm.mode & 0x80);
    buffer[2] = decToBcd(alarm.hour) | (alarm.mode & 0x80);
    buffer[3] = decToBcd(alarm.day) | (alarm.mode & 0x80);
    
    if (!writeRegisters(DS3231_REG_ALARM1_SEC, buffer, 4)) {
        return false;
    }
    
    uint8_t ctrl;
    readRegister(DS3231_REG_CONTROL, &ctrl);
    if (alarm.enabled) {
        ctrl |= 0x01;  // A1IE
    } else {
        ctrl &= ~0x01;
    }
    return writeRegister(DS3231_REG_CONTROL, ctrl);
}

bool DS3231Driver::checkAlarm1() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(DS3231_REG_STATUS, &status)) {
        return false;
    }
    
    return (status & 0x01) != 0;
}

bool DS3231Driver::clearAlarm1() {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!readRegister(DS3231_REG_STATUS, &status)) {
        return false;
    }
    
    status &= ~0x01;
    return writeRegister(DS3231_REG_STATUS, status);
}

bool DS3231Driver::setAgingOffset(int8_t offset) {
    if (!initialized) {
        return false;
    }
    
    return writeRegister(DS3231_REG_AGING, (uint8_t)offset);
}

int8_t DS3231Driver::getAgingOffset() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t offset;
    if (!readRegister(DS3231_REG_AGING, &offset)) {
        return 0;
    }
    
    return (int8_t)offset;
}

bool DS3231Driver::setSquareWave(bool enable, uint8_t rate) {
    if (!initialized || rate > 3) {
        return false;
    }
    
    uint8_t ctrl;
    if (!readRegister(DS3231_REG_CONTROL, &ctrl)) {
        return false;
    }
    
    if (enable) {
        ctrl &= ~0x04;  // Clear INTCN
        ctrl = (ctrl & ~0x18) | ((rate & 0x03) << 3);
    } else {
        ctrl |= 0x04;  // Set INTCN
    }
    
    return writeRegister(DS3231_REG_CONTROL, ctrl);
}
#endif

CapabilitySchema DS3231Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ds3231";
    schema.tier = POCKETOS_DS3231_TIER_NAME;
    schema.description = "DS3231 Precision RTC with Temperature";
    schema.capabilities = "datetime_read,datetime_write,temperature_read";
    
#if POCKETOS_DS3231_ENABLE_ALARM_FEATURES
    schema.capabilities += ",alarm,calibration,square_wave";
#endif
    
    return schema;
}

String DS3231Driver::getParameter(const String& name) {
    if (name == "time") {
        DS3231DateTime dt = readDateTime();
        if (dt.valid) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            return String(buf);
        }
    } else if (name == "temperature") {
        float temp = readTemperature();
        return String(temp, 2);
    }
    return "";
}

bool DS3231Driver::setParameter(const String& name, const String& value) {
    if (name == "time" && value.length() >= 19) {
        DS3231DateTime dt;
        dt.year = value.substring(0, 4).toInt();
        dt.month = value.substring(5, 7).toInt();
        dt.day = value.substring(8, 10).toInt();
        dt.hour = value.substring(11, 13).toInt();
        dt.minute = value.substring(14, 16).toInt();
        dt.second = value.substring(17, 19).toInt();
        dt.dayOfWeek = 1;
        return setDateTime(dt);
    }
    return false;
}

#if POCKETOS_DS3231_ENABLE_REGISTER_ACCESS
const RegisterDesc* DS3231Driver::registers(size_t& count) const {
    count = DS3231_REGISTER_COUNT;
    return DS3231_REGISTERS;
}

bool DS3231Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x12 || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        DS3231_REGISTERS, DS3231_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool DS3231Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x12 || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        DS3231_REGISTERS, DS3231_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* DS3231Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(DS3231_REGISTERS, DS3231_REGISTER_COUNT, name);
}
#endif

bool DS3231Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool DS3231Driver::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length) {
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

bool DS3231Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool DS3231Driver::writeRegisters(uint8_t reg, const uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(buffer[i]);
    }
    return Wire.endTransmission() == 0;
}

uint8_t DS3231Driver::bcdToDec(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

uint8_t DS3231Driver::decToBcd(uint8_t val) {
    return ((val / 10) * 16) + (val % 10);
}

} // namespace PocketOS
