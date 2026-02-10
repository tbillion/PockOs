#include "mcp79410_driver.h"
#include "../driver_config.h"

#if POCKETOS_MCP79410_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// MCP79410 Register addresses
#define MCP79410_REG_RTCSEC       0x00
#define MCP79410_REG_RTCMIN       0x01
#define MCP79410_REG_RTCHOUR      0x02
#define MCP79410_REG_RTCWKDAY     0x03
#define MCP79410_REG_RTCDATE      0x04
#define MCP79410_REG_RTCMTH       0x05
#define MCP79410_REG_RTCYEAR      0x06
#define MCP79410_REG_CONTROL      0x07
#define MCP79410_REG_OSCTRIM      0x08
#define MCP79410_REG_ALM0SEC      0x0A
#define MCP79410_REG_ALM0MIN      0x0B
#define MCP79410_REG_ALM0HOUR     0x0C
#define MCP79410_REG_ALM0WKDAY    0x0D
#define MCP79410_REG_ALM0DATE     0x0E
#define MCP79410_REG_ALM0MTH      0x0F
#define MCP79410_REG_ALM1SEC      0x11
#define MCP79410_REG_ALM1MIN      0x12
#define MCP79410_REG_ALM1HOUR     0x13
#define MCP79410_REG_ALM1WKDAY    0x14
#define MCP79410_REG_ALM1DATE     0x15
#define MCP79410_REG_ALM1MTH      0x16
#define MCP79410_REG_PWRDNMIN     0x18
#define MCP79410_REG_PWRDNHOUR    0x19
#define MCP79410_REG_PWRDNDATE    0x1A
#define MCP79410_REG_PWRDNMTH     0x1B
#define MCP79410_REG_PWRUPMIN     0x1C
#define MCP79410_REG_PWRUPHOUR    0x1D
#define MCP79410_REG_PWRUPDATE    0x1E
#define MCP79410_REG_PWRUPMTH     0x1F
#define MCP79410_REG_SRAM_START   0x20
#define MCP79410_REG_SRAM_END     0x5F

// Bit definitions
#define MCP79410_ST_BIT           0x80
#define MCP79410_VBATEN_BIT       0x08
#define MCP79410_OSCRUN_BIT       0x20
#define MCP79410_PWRFAIL_BIT      0x10
#define MCP79410_VBAT_BIT         0x04

#if POCKETOS_MCP79410_ENABLE_REGISTER_ACCESS
// Complete register map
static const RegisterDesc MCP79410_REGISTERS[] = {
    RegisterDesc(0x00, "RTCSEC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "RTCMIN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "RTCHOUR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "RTCWKDAY", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x04, "RTCDATE", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x05, "RTCMTH", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x06, "RTCYEAR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "OSCTRIM", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "RESERVED_09", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x0A, "ALM0SEC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "ALM0MIN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "ALM0HOUR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "ALM0WKDAY", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "ALM0DATE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0F, "ALM0MTH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x10, "RESERVED_10", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x11, "ALM1SEC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "ALM1MIN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "ALM1HOUR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x14, "ALM1WKDAY", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x15, "ALM1DATE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x16, "ALM1MTH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x17, "RESERVED_17", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x18, "PWRDNMIN", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x19, "PWRDNHOUR", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1A, "PWRDNDATE", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1B, "PWRDNMTH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1C, "PWRUPMIN", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1D, "PWRUPHOUR", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1E, "PWRUPDATE", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1F, "PWRUPMTH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x20, "SRAM_00", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x21, "SRAM_01", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "SRAM_02", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x23, "SRAM_03", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x24, "SRAM_04", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x25, "SRAM_05", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x26, "SRAM_06", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x27, "SRAM_07", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "SRAM_08", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x29, "SRAM_09", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2A, "SRAM_0A", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2B, "SRAM_0B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2C, "SRAM_0C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2D, "SRAM_0D", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2E, "SRAM_0E", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2F, "SRAM_0F", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x30, "SRAM_10", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x31, "SRAM_11", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x32, "SRAM_12", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x33, "SRAM_13", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x34, "SRAM_14", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x35, "SRAM_15", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x36, "SRAM_16", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x37, "SRAM_17", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x38, "SRAM_18", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x39, "SRAM_19", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3A, "SRAM_1A", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3B, "SRAM_1B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3C, "SRAM_1C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3D, "SRAM_1D", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3E, "SRAM_1E", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3F, "SRAM_1F", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x40, "SRAM_20", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x41, "SRAM_21", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x42, "SRAM_22", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x43, "SRAM_23", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x44, "SRAM_24", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x45, "SRAM_25", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x46, "SRAM_26", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x47, "SRAM_27", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x48, "SRAM_28", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x49, "SRAM_29", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4A, "SRAM_2A", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4B, "SRAM_2B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4C, "SRAM_2C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4D, "SRAM_2D", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4E, "SRAM_2E", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4F, "SRAM_2F", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x50, "SRAM_30", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x51, "SRAM_31", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x52, "SRAM_32", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x53, "SRAM_33", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x54, "SRAM_34", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x55, "SRAM_35", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x56, "SRAM_36", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x57, "SRAM_37", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x58, "SRAM_38", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x59, "SRAM_39", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5A, "SRAM_3A", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5B, "SRAM_3B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5C, "SRAM_3C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5D, "SRAM_3D", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5E, "SRAM_3E", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5F, "SRAM_3F", 1, RegisterAccess::RW, 0x00),
};

#define MCP79410_REGISTER_COUNT (sizeof(MCP79410_REGISTERS) / sizeof(RegisterDesc))
#endif

MCP79410Driver::MCP79410Driver() : address(0), initialized(false) {
}

bool MCP79410Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MCP79410_ENABLE_LOGGING
    Logger::info("MCP79410: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t seconds = 0;
    if (!readRegister(MCP79410_REG_RTCSEC, &seconds)) {
#if POCKETOS_MCP79410_ENABLE_LOGGING
        Logger::error("MCP79410: Failed to communicate with device");
#endif
        return false;
    }
    
#if POCKETOS_MCP79410_ENABLE_CONFIGURATION
    // Start oscillator if not running
    if (!(seconds & MCP79410_ST_BIT)) {
        seconds |= MCP79410_ST_BIT;
        if (!writeRegister(MCP79410_REG_RTCSEC, seconds)) {
#if POCKETOS_MCP79410_ENABLE_LOGGING
            Logger::error("MCP79410: Failed to start oscillator");
#endif
            return false;
        }
    }
    
    // Enable battery backup by default
    uint8_t wkday;
    if (readRegister(MCP79410_REG_RTCWKDAY, &wkday)) {
        if (!(wkday & MCP79410_VBATEN_BIT)) {
            wkday |= MCP79410_VBATEN_BIT;
            writeRegister(MCP79410_REG_RTCWKDAY, wkday);
        }
    }
#endif
    
    initialized = true;
#if POCKETOS_MCP79410_ENABLE_LOGGING
    Logger::info("MCP79410: Initialized successfully");
#endif
    return true;
}

void MCP79410Driver::deinit() {
    initialized = false;
#if POCKETOS_MCP79410_ENABLE_LOGGING
    Logger::info("MCP79410: Deinitialized");
#endif
}

MCP79410DateTime MCP79410Driver::readDateTime() {
    MCP79410DateTime dt;
    
    if (!initialized) {
        return dt;
    }
    
    uint8_t buffer[7];
    if (!readRegisters(MCP79410_REG_RTCSEC, buffer, 7)) {
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

bool MCP79410Driver::setDateTime(const MCP79410DateTime& dt) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[7];
    buffer[0] = decToBcd(dt.second) | MCP79410_ST_BIT;
    buffer[1] = decToBcd(dt.minute);
    buffer[2] = decToBcd(dt.hour);
    buffer[3] = decToBcd(dt.dayOfWeek) | MCP79410_VBATEN_BIT;
    buffer[4] = decToBcd(dt.day);
    buffer[5] = decToBcd(dt.month);
    buffer[6] = decToBcd(dt.year >= 2000 ? dt.year - 2000 : dt.year);
    
    return writeRegisters(MCP79410_REG_RTCSEC, buffer, 7);
}

#if POCKETOS_MCP79410_ENABLE_ALARM_FEATURES
bool MCP79410Driver::setAlarm0(const MCP79410Alarm& alarm) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[6];
    buffer[0] = decToBcd(alarm.second);
    buffer[1] = decToBcd(alarm.minute);
    buffer[2] = decToBcd(alarm.hour);
    buffer[3] = decToBcd(alarm.dayOfWeek) | (alarm.mask << 4);
    buffer[4] = decToBcd(alarm.day);
    buffer[5] = decToBcd(alarm.month) | (alarm.enabled ? 0x10 : 0x00);
    
    return writeRegisters(MCP79410_REG_ALM0SEC, buffer, 6);
}

bool MCP79410Driver::setAlarm1(const MCP79410Alarm& alarm) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[6];
    buffer[0] = decToBcd(alarm.second);
    buffer[1] = decToBcd(alarm.minute);
    buffer[2] = decToBcd(alarm.hour);
    buffer[3] = decToBcd(alarm.dayOfWeek) | (alarm.mask << 4);
    buffer[4] = decToBcd(alarm.day);
    buffer[5] = decToBcd(alarm.month) | (alarm.enabled ? 0x10 : 0x00);
    
    return writeRegisters(MCP79410_REG_ALM1SEC, buffer, 6);
}

MCP79410Alarm MCP79410Driver::getAlarm0() {
    MCP79410Alarm alarm;
    if (!initialized) {
        return alarm;
    }
    
    uint8_t buffer[6];
    if (readRegisters(MCP79410_REG_ALM0SEC, buffer, 6)) {
        alarm.second = bcdToDec(buffer[0] & 0x7F);
        alarm.minute = bcdToDec(buffer[1] & 0x7F);
        alarm.hour = bcdToDec(buffer[2] & 0x3F);
        alarm.dayOfWeek = bcdToDec(buffer[3] & 0x07);
        alarm.mask = (buffer[3] >> 4) & 0x07;
        alarm.day = bcdToDec(buffer[4] & 0x3F);
        alarm.month = bcdToDec(buffer[5] & 0x1F);
        alarm.enabled = (buffer[5] & 0x10) != 0;
    }
    
    return alarm;
}

MCP79410Alarm MCP79410Driver::getAlarm1() {
    MCP79410Alarm alarm;
    if (!initialized) {
        return alarm;
    }
    
    uint8_t buffer[6];
    if (readRegisters(MCP79410_REG_ALM1SEC, buffer, 6)) {
        alarm.second = bcdToDec(buffer[0] & 0x7F);
        alarm.minute = bcdToDec(buffer[1] & 0x7F);
        alarm.hour = bcdToDec(buffer[2] & 0x3F);
        alarm.dayOfWeek = bcdToDec(buffer[3] & 0x07);
        alarm.mask = (buffer[3] >> 4) & 0x07;
        alarm.day = bcdToDec(buffer[4] & 0x3F);
        alarm.month = bcdToDec(buffer[5] & 0x1F);
        alarm.enabled = (buffer[5] & 0x10) != 0;
    }
    
    return alarm;
}

bool MCP79410Driver::checkAlarm0() {
    if (!initialized) {
        return false;
    }
    
    uint8_t wkday;
    if (!readRegister(MCP79410_REG_ALM0WKDAY, &wkday)) {
        return false;
    }
    
    return (wkday & 0x08) != 0;
}

bool MCP79410Driver::checkAlarm1() {
    if (!initialized) {
        return false;
    }
    
    uint8_t wkday;
    if (!readRegister(MCP79410_REG_ALM1WKDAY, &wkday)) {
        return false;
    }
    
    return (wkday & 0x08) != 0;
}

bool MCP79410Driver::clearAlarm0() {
    if (!initialized) {
        return false;
    }
    
    uint8_t wkday;
    if (!readRegister(MCP79410_REG_ALM0WKDAY, &wkday)) {
        return false;
    }
    
    wkday &= ~0x08;
    return writeRegister(MCP79410_REG_ALM0WKDAY, wkday);
}

bool MCP79410Driver::clearAlarm1() {
    if (!initialized) {
        return false;
    }
    
    uint8_t wkday;
    if (!readRegister(MCP79410_REG_ALM1WKDAY, &wkday)) {
        return false;
    }
    
    wkday &= ~0x08;
    return writeRegister(MCP79410_REG_ALM1WKDAY, wkday);
}

bool MCP79410Driver::readSRAM(uint8_t offset, uint8_t* data, uint8_t length) {
    if (!initialized || offset >= 64 || (offset + length) > 64) {
        return false;
    }
    
    return readRegisters(MCP79410_REG_SRAM_START + offset, data, length);
}

bool MCP79410Driver::writeSRAM(uint8_t offset, const uint8_t* data, uint8_t length) {
    if (!initialized || offset >= 64 || (offset + length) > 64) {
        return false;
    }
    
    return writeRegisters(MCP79410_REG_SRAM_START + offset, data, length);
}

bool MCP79410Driver::getPowerFailTimestamp(MCP79410DateTime& dt) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[4];
    if (!readRegisters(MCP79410_REG_PWRDNMIN, buffer, 4)) {
        return false;
    }
    
    dt.minute = bcdToDec(buffer[0] & 0x7F);
    dt.hour = bcdToDec(buffer[1] & 0x3F);
    dt.day = bcdToDec(buffer[2] & 0x3F);
    dt.month = bcdToDec(buffer[3] & 0x1F);
    dt.dayOfWeek = (buffer[3] >> 5) & 0x07;
    dt.valid = true;
    
    return true;
}

bool MCP79410Driver::getPowerRestoreTimestamp(MCP79410DateTime& dt) {
    if (!initialized) {
        return false;
    }
    
    uint8_t buffer[4];
    if (!readRegisters(MCP79410_REG_PWRUPMIN, buffer, 4)) {
        return false;
    }
    
    dt.minute = bcdToDec(buffer[0] & 0x7F);
    dt.hour = bcdToDec(buffer[1] & 0x3F);
    dt.day = bcdToDec(buffer[2] & 0x3F);
    dt.month = bcdToDec(buffer[3] & 0x1F);
    dt.dayOfWeek = (buffer[3] >> 5) & 0x07;
    dt.valid = true;
    
    return true;
}

bool MCP79410Driver::enableBatteryBackup(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint8_t wkday;
    if (!readRegister(MCP79410_REG_RTCWKDAY, &wkday)) {
        return false;
    }
    
    if (enable) {
        wkday |= MCP79410_VBATEN_BIT;
    } else {
        wkday &= ~MCP79410_VBATEN_BIT;
    }
    
    return writeRegister(MCP79410_REG_RTCWKDAY, wkday);
}

bool MCP79410Driver::setSquareWave(bool enable, uint8_t freq) {
    if (!initialized || freq > 3) {
        return false;
    }
    
    uint8_t ctrl;
    if (!readRegister(MCP79410_REG_CONTROL, &ctrl)) {
        return false;
    }
    
    if (enable) {
        ctrl = (ctrl & ~0x03) | (freq & 0x03);
        ctrl |= 0x40;  // Enable square wave output
    } else {
        ctrl &= ~0x40;
    }
    
    return writeRegister(MCP79410_REG_CONTROL, ctrl);
}

bool MCP79410Driver::setCalibration(int8_t trim) {
    if (!initialized) {
        return false;
    }
    
    return writeRegister(MCP79410_REG_OSCTRIM, (uint8_t)trim);
}

int8_t MCP79410Driver::getCalibration() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t trim;
    if (!readRegister(MCP79410_REG_OSCTRIM, &trim)) {
        return 0;
    }
    
    return (int8_t)trim;
}
#endif

CapabilitySchema MCP79410Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "mcp79410";
    schema.tier = POCKETOS_MCP79410_TIER_NAME;
    schema.description = "MCP79410 RTC with Battery Backup and SRAM";
    schema.capabilities = "datetime_read,datetime_write";
    
#if POCKETOS_MCP79410_ENABLE_ALARM_FEATURES
    schema.capabilities += ",dual_alarm,sram_access,power_fail_timestamp,square_wave,calibration";
#endif
    
    return schema;
}

String MCP79410Driver::getParameter(const String& name) {
    if (name == "time") {
        MCP79410DateTime dt = readDateTime();
        if (dt.valid) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            return String(buf);
        }
    }
    return "";
}

bool MCP79410Driver::setParameter(const String& name, const String& value) {
    if (name == "time" && value.length() >= 19) {
        MCP79410DateTime dt;
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

#if POCKETOS_MCP79410_ENABLE_REGISTER_ACCESS
const RegisterDesc* MCP79410Driver::registers(size_t& count) const {
    count = MCP79410_REGISTER_COUNT;
    return MCP79410_REGISTERS;
}

bool MCP79410Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x5F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        MCP79410_REGISTERS, MCP79410_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool MCP79410Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x5F || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        MCP79410_REGISTERS, MCP79410_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MCP79410Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MCP79410_REGISTERS, MCP79410_REGISTER_COUNT, name);
}
#endif

bool MCP79410Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool MCP79410Driver::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t length) {
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

bool MCP79410Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MCP79410Driver::writeRegisters(uint8_t reg, const uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(buffer[i]);
    }
    return Wire.endTransmission() == 0;
}

uint8_t MCP79410Driver::bcdToDec(uint8_t val) {
    return ((val / 16) * 10) + (val % 16);
}

uint8_t MCP79410Driver::decToBcd(uint8_t val) {
    return ((val / 10) * 16) + (val % 10);
}

} // namespace PocketOS
