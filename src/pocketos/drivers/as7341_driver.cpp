#include "as7341_driver.h"
#include "../driver_config.h"

#if POCKETOS_AS7341_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define AS7341_REG_ENABLE      0x80
#define AS7341_REG_ATIME       0x81
#define AS7341_REG_ASTEP_L     0x CA
#define AS7341_REG_ASTEP_H     0xCB
#define AS7341_REG_CFG0        0xA9
#define AS7341_REG_CFG1        0xAA
#define AS7341_REG_CFG6        0xAF
#define AS7341_REG_STATUS      0x93
#define AS7341_REG_CH0_DATA_L  0x95

#if POCKETOS_AS7341_ENABLE_REGISTER_ACCESS
static const RegisterDesc AS7341_REGISTERS[] = {
    RegisterDesc(0x80, "ENABLE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x81, "ATIME", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x93, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x95, "CH0_DATA_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x96, "CH0_DATA_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xA9, "CFG0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xAA, "CFG1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xAF, "CFG6", 1, RegisterAccess::RW, 0x10),
    RegisterDesc(0xCA, "ASTEP_L", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xCB, "ASTEP_H", 1, RegisterAccess::RW, 0x00),
};

#define AS7341_REGISTER_COUNT (sizeof(AS7341_REGISTERS) / sizeof(RegisterDesc))
#endif

AS7341Driver::AS7341Driver() : address(0), initialized(false) {}

bool AS7341Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AS7341_ENABLE_LOGGING
    Logger::info("AS7341: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_AS7341_ENABLE_CONFIGURATION
    writeRegister(AS7341_REG_ENABLE, 0x01);
    writeRegister(AS7341_REG_ATIME, 0x64);
#endif
    
    initialized = true;
#if POCKETOS_AS7341_ENABLE_LOGGING
    Logger::info("AS7341: Initialized successfully");
#endif
    return true;
}

void AS7341Driver::deinit() {
    if (initialized) {
        writeRegister(AS7341_REG_ENABLE, 0x00);
    }
    initialized = false;
}

AS7341Data AS7341Driver::readData() {
    AS7341Data data;
    
    if (!initialized) {
        return data;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema AS7341Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_AS7341_TIER_NAME;
    return schema;
}

bool AS7341Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool AS7341Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        *value = Wire.read();
        return true;
    }
    return false;
}

#if POCKETOS_AS7341_ENABLE_REGISTER_ACCESS
const RegisterDesc* AS7341Driver::registers(size_t& count) const {
    count = AS7341_REGISTER_COUNT;
    return AS7341_REGISTERS;
}

bool AS7341Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AS7341_REGISTERS, AS7341_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool AS7341Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AS7341_REGISTERS, AS7341_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* AS7341Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(AS7341_REGISTERS, AS7341_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
