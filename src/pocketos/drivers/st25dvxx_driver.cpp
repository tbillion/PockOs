#include "st25dvxx_driver.h"
#include "../driver_config.h"

#if POCKETOS_ST25DVXX_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define ST25DVXX_REG_CONTROL    0x00
#define ST25DVXX_REG_STATUS     0x01
#define ST25DVXX_REG_CONFIG     0x02

#if POCKETOS_ST25DVXX_ENABLE_REGISTER_ACCESS
static const RegisterDesc ST25DVXX_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "CONFIG", 1, RegisterAccess::RW, 0x00),
};

#define ST25DVXX_REGISTER_COUNT (sizeof(ST25DVXX_REGISTERS) / sizeof(RegisterDesc))
#endif

ST25DVxxDriver::ST25DVxxDriver() : address(0), initialized(false) {}

bool ST25DVxxDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_ST25DVXX_ENABLE_LOGGING
    Logger::info("ST25DVxx: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_ST25DVXX_ENABLE_CONFIGURATION
    writeRegister(ST25DVXX_REG_CONTROL, 0x00);
    writeRegister(ST25DVXX_REG_CONFIG, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_ST25DVXX_ENABLE_LOGGING
    Logger::info("ST25DVxx: Initialized successfully");
#endif
    return true;
}

void ST25DVxxDriver::deinit() {
    initialized = false;
}

ST25DVxxData ST25DVxxDriver::readData() {
    ST25DVxxData data;
    
    if (!initialized) {
        return data;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema ST25DVxxDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_ST25DVXX_TIER_NAME;
    return schema;
}

bool ST25DVxxDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool ST25DVxxDriver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_ST25DVXX_ENABLE_REGISTER_ACCESS
const RegisterDesc* ST25DVxxDriver::registers(size_t& count) const {
    count = ST25DVXX_REGISTER_COUNT;
    return ST25DVXX_REGISTERS;
}

bool ST25DVxxDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(ST25DVXX_REGISTERS, ST25DVXX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool ST25DVxxDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(ST25DVXX_REGISTERS, ST25DVXX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* ST25DVxxDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(ST25DVXX_REGISTERS, ST25DVXX_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
