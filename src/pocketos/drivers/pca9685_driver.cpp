#include "pca9685_driver.h"
#include "../driver_config.h"

#if POCKETOS_PCA9685_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define PCA9685_REG_MODE1      0x00
#define PCA9685_REG_MODE2      0x01
#define PCA9685_REG_LED0_ON_L  0x06

#if POCKETOS_PCA9685_ENABLE_REGISTER_ACCESS
static const RegisterDesc PCA9685_REGISTERS[] = {
    RegisterDesc(0x00, "MODE1", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x01, "MODE2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "LED0_ON_L", 1, RegisterAccess::RW, 0x00),
};

#define PCA9685_REGISTER_COUNT (sizeof(PCA9685_REGISTERS) / sizeof(RegisterDesc))
#endif

PCA9685Driver::PCA9685Driver() : address(0), initialized(false) {}

bool PCA9685Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_PCA9685_ENABLE_LOGGING
    Logger::info("PCA9685: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_PCA9685_ENABLE_CONFIGURATION
    writeRegister(PCA9685_REG_MODE1, 0x00);
    delay(5);
    writeRegister(PCA9685_REG_MODE2, 0x04);
#endif
    
    initialized = true;
#if POCKETOS_PCA9685_ENABLE_LOGGING
    Logger::info("PCA9685: Initialized successfully");
#endif
    return true;
}

void PCA9685Driver::deinit() {
    if (initialized) {
        writeRegister(PCA9685_REG_MODE1, 0x10);
    }
    initialized = false;
}

bool PCA9685Driver::setPWM(uint8_t channel, uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    uint8_t reg = PCA9685_REG_LED0_ON_L + 4 * channel;
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(0);
    Wire.write(0);
    Wire.write(value & 0xFF);
    Wire.write(value >> 8);
    return Wire.endTransmission() == 0;
}

CapabilitySchema PCA9685Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_PCA9685_TIER_NAME;
    return schema;
}

bool PCA9685Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool PCA9685Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_PCA9685_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCA9685Driver::registers(size_t& count) const {
    count = PCA9685_REGISTER_COUNT;
    return PCA9685_REGISTERS;
}

bool PCA9685Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(PCA9685_REGISTERS, PCA9685_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool PCA9685Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(PCA9685_REGISTERS, PCA9685_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* PCA9685Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCA9685_REGISTERS, PCA9685_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
