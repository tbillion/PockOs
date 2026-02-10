#include "lc709203f_driver.h"
#include "../driver_config.h"

#if POCKETOS_LC709203F_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define LC709203F_REG_CELL_VOLTAGE   0x09
#define LC709203F_REG_RSOC           0x0D
#define LC709203F_REG_ITE            0x0F
#define LC709203F_REG_IC_VERSION     0x11
#define LC709203F_REG_BATT_PROFILE   0x12
#define LC709203F_REG_ALARM_LOW_RSOC 0x13
#define LC709203F_REG_ALARM_LOW_VOLT 0x14
#define LC709203F_REG_IC_POWER_MODE  0x15
#define LC709203F_REG_STATUS_BIT     0x16

#if POCKETOS_LC709203F_ENABLE_REGISTER_ACCESS
static const RegisterDesc LC709203F_REGISTERS[] = {
    RegisterDesc(0x09, "CELL_VOLTAGE", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x0D, "RSOC", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x0F, "ITE", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x11, "IC_VERSION", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x12, "BATT_PROFILE", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x13, "ALARM_LOW_RSOC", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x14, "ALARM_LOW_VOLT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x15, "IC_POWER_MODE", 2, RegisterAccess::RW, 0x0001),
    RegisterDesc(0x16, "STATUS_BIT", 2, RegisterAccess::RW, 0x0000),
};

#define LC709203F_REGISTER_COUNT (sizeof(LC709203F_REGISTERS) / sizeof(RegisterDesc))
#endif

LC709203FDriver::LC709203FDriver() : address(0), initialized(false) {}

bool LC709203FDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_LC709203F_ENABLE_LOGGING
    Logger::info("LC709203F: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint16_t version = 0;
    if (!readRegister(LC709203F_REG_IC_VERSION, &version)) {
#if POCKETOS_LC709203F_ENABLE_LOGGING
        Logger::error("LC709203F: Failed to read version");
#endif
        return false;
    }
    
#if POCKETOS_LC709203F_ENABLE_CONFIGURATION
    writeRegister(LC709203F_REG_IC_POWER_MODE, 0x0001);
#endif
    
    initialized = true;
#if POCKETOS_LC709203F_ENABLE_LOGGING
    Logger::info("LC709203F: Initialized successfully");
#endif
    return true;
}

void LC709203FDriver::deinit() {
    if (initialized) {
        writeRegister(LC709203F_REG_IC_POWER_MODE, 0x0002);
    }
    initialized = false;
}

LC709203FData LC709203FDriver::readData() {
    LC709203FData data;
    
    if (!initialized) {
        return data;
    }
    
    uint16_t voltage_raw = 0, rsoc = 0;
    if (readRegister(LC709203F_REG_CELL_VOLTAGE, &voltage_raw) &&
        readRegister(LC709203F_REG_RSOC, &rsoc)) {
        data.voltage = voltage_raw / 1000.0;
        data.percentage = rsoc / 10.0;
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema LC709203FDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_LC709203F_TIER_NAME;
    return schema;
}

bool LC709203FDriver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((uint8_t)(value & 0xFF));
    Wire.write((uint8_t)(value >> 8));
    return Wire.endTransmission() == 0;
}

bool LC709203FDriver::readRegister(uint8_t reg, uint16_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)2);
    if (Wire.available() >= 2) {
        uint8_t low = Wire.read();
        uint8_t high = Wire.read();
        *value = (high << 8) | low;
        return true;
    }
    return false;
}

#if POCKETOS_LC709203F_ENABLE_REGISTER_ACCESS
const RegisterDesc* LC709203FDriver::registers(size_t& count) const {
    count = LC709203F_REGISTER_COUNT;
    return LC709203F_REGISTERS;
}

bool LC709203FDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(LC709203F_REGISTERS, LC709203F_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    uint16_t value;
    if (readRegister((uint8_t)reg, &value)) {
        buf[0] = value & 0xFF;
        buf[1] = value >> 8;
        return true;
    }
    return false;
}

bool LC709203FDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(LC709203F_REGISTERS, LC709203F_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    uint16_t value = buf[0] | (buf[1] << 8);
    return writeRegister((uint8_t)reg, value);
}

const RegisterDesc* LC709203FDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(LC709203F_REGISTERS, LC709203F_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
