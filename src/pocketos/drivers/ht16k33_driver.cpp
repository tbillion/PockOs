#include "ht16k33_driver.h"
#include "../driver_config.h"

#if POCKETOS_HT16K33_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define HT16K33_REG_MODE1      0x00
#define HT16K33_REG_MODE2      0x01
#define HT16K33_REG_LED0_ON_L  0x06

#if POCKETOS_HT16K33_ENABLE_REGISTER_ACCESS
static const RegisterDesc HT16K33_REGISTERS[] = {
    RegisterDesc(0x00, "MODE1", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x01, "MODE2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "LED0_ON_L", 1, RegisterAccess::RW, 0x00),
};

#define HT16K33_REGISTER_COUNT (sizeof(HT16K33_REGISTERS) / sizeof(RegisterDesc))
#endif

HT16K33Driver::HT16K33Driver() : address(0), initialized(false) {}

bool HT16K33Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_HT16K33_ENABLE_LOGGING
    Logger::info("HT16K33: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_HT16K33_ENABLE_CONFIGURATION
    writeRegister(HT16K33_REG_MODE1, 0x00);
    delay(5);
    writeRegister(HT16K33_REG_MODE2, 0x04);
#endif
    
    initialized = true;
#if POCKETOS_HT16K33_ENABLE_LOGGING
    Logger::info("HT16K33: Initialized successfully");
#endif
    return true;
}

void HT16K33Driver::deinit() {
    if (initialized) {
        writeRegister(HT16K33_REG_MODE1, 0x10);
    }
    initialized = false;
}

bool HT16K33Driver::setPWM(uint8_t channel, uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    uint8_t reg = HT16K33_REG_LED0_ON_L + 4 * channel;
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(0);
    Wire.write(0);
    Wire.write(value & 0xFF);
    Wire.write(value >> 8);
    return Wire.endTransmission() == 0;
}

CapabilitySchema HT16K33Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_HT16K33_TIER_NAME;
    return schema;
}

bool HT16K33Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool HT16K33Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_HT16K33_ENABLE_REGISTER_ACCESS
const RegisterDesc* HT16K33Driver::registers(size_t& count) const {
    count = HT16K33_REGISTER_COUNT;
    return HT16K33_REGISTERS;
}

bool HT16K33Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(HT16K33_REGISTERS, HT16K33_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool HT16K33Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(HT16K33_REGISTERS, HT16K33_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* HT16K33Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(HT16K33_REGISTERS, HT16K33_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
