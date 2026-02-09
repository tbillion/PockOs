#include "ds1307_driver.h"
#include "../driver_config.h"

#if POCKETOS_DS1307_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// DS1307 Register addresses
#define DS1307_REG_SECONDS     0x00
#define DS1307_REG_MINUTES     0x01
#define DS1307_REG_HOURS       0x02
#define DS1307_REG_DAY         0x03
#define DS1307_REG_DATE        0x04
#define DS1307_REG_MONTH       0x05
#define DS1307_REG_YEAR        0x06
#define DS1307_REG_CONTROL     0x07
#define DS1307_REG_SRAM_START  0x08
#define DS1307_REG_SRAM_END    0x3F

// Control register bits
#define DS1307_CTRL_OUT        0x80
#define DS1307_CTRL_SQWE       0x10
#define DS1307_CTRL_RS1        0x02
#define DS1307_CTRL_RS0        0x01

#if POCKETOS_DS1307_ENABLE_REGISTER_ACCESS
// Complete register map
static const RegisterDesc DS1307_REGISTERS[] = {
    RegisterDesc(0x00, "SECONDS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "MINUTES", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "HOURS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "DAY", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x04, "DATE", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x05, "MONTH", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x06, "YEAR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "CONTROL", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x08, "RAM_00", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "RAM_01", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "RAM_02", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "RAM_03", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "RAM_04", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "RAM_05", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "RAM_06", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0F, "RAM_07", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x10, "RAM_08", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "RAM_09", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "RAM_0A", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "RAM_0B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x14, "RAM_0C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x15, "RAM_0D", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x16, "RAM_0E", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x17, "RAM_0F", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x18, "RAM_10", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x19, "RAM_11", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1A, "RAM_12", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1B, "RAM_13", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1C, "RAM_14", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1D, "RAM_15", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1E, "RAM_16", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1F, "RAM_17", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x20, "RAM_18", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x21, "RAM_19", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "RAM_1A", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x23, "RAM_1B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x24, "RAM_1C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x25, "RAM_1D", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x26, "RAM_1E", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x27, "RAM_1F", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "RAM_20", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x29, "RAM_21", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2A, "RAM_22", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2B, "RAM_23", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2C, "RAM_24", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2D, "RAM_25", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2E, "RAM_26", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2F, "RAM_27", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x30, "RAM_28", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x31, "RAM_29", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x32, "RAM_2A", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x33, "RAM_2B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x34, "RAM_2C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x35, "RAM_2D", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x36, "RAM_2E", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x37, "RAM_2F", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x38, "RAM_30", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x39, "RAM_31", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3A, "RAM_32", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3B, "RAM_33", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3C, "RAM_34", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3D, "RAM_35", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3E, "RAM_36", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3F, "RAM_37", 1, RegisterAccess::RW, 0x00),
};

#define DS1307_REGISTER_COUNT (sizeof(DS1307_REGISTERS) / sizeof(RegisterDesc))
#endif

DS1307Driver::DS1307Driver() : address(0), initialized(false) {
}

bool DS1307Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_DS1307_ENABLE_LOGGING
    Logger::info("DS1307: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check if device is present by reading seconds register
    uint8_t seconds = 0;
    if (!readRegister(DS1307_REG_SECONDS, &seconds)) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Failed to communicate with device");
#endif
        return false;
    }
    
#if POCKETOS_DS1307_ENABLE_CONFIGURATION
    // Enable oscillator (clear CH bit in seconds register)
    seconds &= 0x7F;
    if (!writeRegister(DS1307_REG_SECONDS, seconds)) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Failed to enable oscillator");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_DS1307_ENABLE_LOGGING
    Logger::info("DS1307: Initialized successfully");
#endif
    return true;
}

void DS1307Driver::deinit() {
    initialized = false;
#if POCKETOS_DS1307_ENABLE_LOGGING
    Logger::info("DS1307: Deinitialized");
#endif
}

DS1307DateTime DS1307Driver::readDateTime() {
    DS1307DateTime dt;
    
    if (!initialized) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Not initialized");
#endif
        return dt;
    }
    
    uint8_t buffer[7];
    if (!readRegisters(DS1307_REG_SECONDS, buffer, 7)) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Failed to read time");
#endif
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

bool DS1307Driver::setDateTime(const DS1307DateTime& dt) {
    if (!initialized) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Not initialized");
#endif
        return false;
    }
    
    uint8_t buffer[7];
    buffer[0] = decToBcd(dt.second) & 0x7F;  // CH bit = 0 (enable oscillator)
    buffer[1] = decToBcd(dt.minute);
    buffer[2] = decToBcd(dt.hour);
    buffer[3] = decToBcd(dt.dayOfWeek);
    buffer[4] = decToBcd(dt.day);
    buffer[5] = decToBcd(dt.month);
    buffer[6] = decToBcd(dt.year >= 2000 ? dt.year - 2000 : dt.year);
    
    if (!writeRegisters(DS1307_REG_SECONDS, buffer, 7)) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Failed to set time");
#endif
        return false;
    }
    
#if POCKETOS_DS1307_ENABLE_LOGGING
    Logger::info("DS1307: Time set successfully");
#endif
    return true;
}

#if POCKETOS_DS1307_ENABLE_ALARM_FEATURES
bool DS1307Driver::readSRAM(uint8_t offset, uint8_t* data, uint8_t length) {
    if (!initialized || offset >= 56 || (offset + length) > 56) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Invalid SRAM parameters");
#endif
        return false;
    }
    
    return readRegisters(DS1307_REG_SRAM_START + offset, data, length);
}

bool DS1307Driver::writeSRAM(uint8_t offset, const uint8_t* data, uint8_t length) {
    if (!initialized || offset >= 56 || (offset + length) > 56) {
#if POCKETOS_DS1307_ENABLE_LOGGING
        Logger::error("DS1307: Invalid SRAM parameters");
#endif
        return false;
    }
    
    return writeRegisters(DS1307_REG_SRAM_START + offset, data, length);
}

bool DS1307Driver::setSquareWave(bool enable, uint8_t rate) {
    if (!initialized || rate > 3) {
        return false;
    }
    
    uint8_t ctrl = 0;
    if (enable) {
        ctrl = DS1307_CTRL_SQWE | (rate & 0x03);
    }
    
    return writeRegister(DS1307_REG_CONTROL, ctrl);
}
#endif

CapabilitySchema DS1307Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ds1307";
    schema.tier = POCKETOS_DS1307_TIER_NAME;
    schema.description = "DS1307 Basic Real-Time Clock";
    schema.capabilities = "datetime_read,datetime_write";
    
#if POCKETOS_DS1307_ENABLE_ALARM_FEATURES
    schema.capabilities += ",sram_access,square_wave";
#endif
    
    return schema;
}

String DS1307Driver::getParameter(const String& name) {
    if (name == "time") {
        DS1307DateTime dt = readDateTime();
        if (dt.valid) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            return String(buf);
        }
    }
    return "";
}

bool DS1307Driver::setParameter(const String& name, const String& value) {
    if (name == "time") {
        // Expected format: "YYYY-MM-DD HH:MM:SS"
        if (value.length() >= 19) {
            DS1307DateTime dt;
            dt.year = value.substring(0, 4).toInt();
            dt.month = value.substring(5, 7).toInt();
            dt.day = value.substring(8, 10).toInt();
            dt.hour = value.substring(11, 13).toInt();
            dt.minute = value.substring(14, 16).toInt();
            dt.second = value.substring(17, 19).toInt();
            dt.dayOfWeek = 1;
            return setDateTime(dt);
        }
    }
    return false;
}

#if POCKETOS_DS1307_ENABLE_REGISTER_ACCESS
const RegisterDesc* DS1307Driver::registers(size_t& count) const {
    count = DS1307_REGISTER_COUNT;
    return DS1307_REGISTERS;
}

bool DS1307Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x3F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        DS1307_REGISTERS, DS1307_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool DS1307Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x3F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        DS1307_REGISTERS, DS1307_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* DS1307Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(DS1307_REGISTERS, DS1307_REGISTER_COUNT, name);
}
#endif

bool DS1307Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool DS1307Driver::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length) {
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

bool DS1307Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool DS1307Driver::writeRegisters(uint8_t reg, const uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(buffer[i]);
    }
    return Wire.endTransmission() == 0;
}

uint8_t DS1307Driver::bcdToDec(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

uint8_t DS1307Driver::decToBcd(uint8_t val) {
    return ((val / 10) * 16) + (val % 10);
}

} // namespace PocketOS
