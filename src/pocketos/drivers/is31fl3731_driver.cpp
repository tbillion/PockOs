#include "is31fl3731_driver.h"
#include "../driver_config.h"

#if POCKETOS_IS31FL3731_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define IS31FL3731_REG_MODE1      0x00
#define IS31FL3731_REG_MODE2      0x01
#define IS31FL3731_REG_LED0_ON_L  0x06

#if POCKETOS_IS31FL3731_ENABLE_REGISTER_ACCESS
static const RegisterDesc IS31FL3731_REGISTERS[] = {
    RegisterDesc(0x00, "MODE1", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x01, "MODE2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "LED0_ON_L", 1, RegisterAccess::RW, 0x00),
};

#define IS31FL3731_REGISTER_COUNT (sizeof(IS31FL3731_REGISTERS) / sizeof(RegisterDesc))
#endif

IS31FL3731Driver::IS31FL3731Driver() : address(0), initialized(false) {}

bool IS31FL3731Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_IS31FL3731_ENABLE_LOGGING
    Logger::info("IS31FL3731: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_IS31FL3731_ENABLE_CONFIGURATION
    writeRegister(IS31FL3731_REG_MODE1, 0x00);
    delay(5);
    writeRegister(IS31FL3731_REG_MODE2, 0x04);
#endif
    
    initialized = true;
#if POCKETOS_IS31FL3731_ENABLE_LOGGING
    Logger::info("IS31FL3731: Initialized successfully");
#endif
    return true;
}

void IS31FL3731Driver::deinit() {
    if (initialized) {
        writeRegister(IS31FL3731_REG_MODE1, 0x10);
    }
    initialized = false;
}

bool IS31FL3731Driver::setPWM(uint8_t channel, uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    uint8_t reg = IS31FL3731_REG_LED0_ON_L + 4 * channel;
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(0);
    Wire.write(0);
    Wire.write(value & 0xFF);
    Wire.write(value >> 8);
    return Wire.endTransmission() == 0;
}

CapabilitySchema IS31FL3731Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_IS31FL3731_TIER_NAME;
    return schema;
}

bool IS31FL3731Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool IS31FL3731Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_IS31FL3731_ENABLE_REGISTER_ACCESS
const RegisterDesc* IS31FL3731Driver::registers(size_t& count) const {
    count = IS31FL3731_REGISTER_COUNT;
    return IS31FL3731_REGISTERS;
}

bool IS31FL3731Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(IS31FL3731_REGISTERS, IS31FL3731_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool IS31FL3731Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(IS31FL3731_REGISTERS, IS31FL3731_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* IS31FL3731Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(IS31FL3731_REGISTERS, IS31FL3731_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
