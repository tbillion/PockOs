#include "ssd1309_driver.h"
#include "../driver_config.h"

#if POCKETOS_SSD1309_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define SSD1309_REG_WHO_AM_I   0x0F
#define SSD1309_REG_CTRL1      0x10
#define SSD1309_REG_CTRL2      0x11
#define SSD1309_REG_STATUS     0x1E

#if POCKETOS_SSD1309_ENABLE_REGISTER_ACCESS
static const RegisterDesc SSD1309_REGISTERS[] = {
    RegisterDesc(0x0F, "WHO_AM_I", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x10, "CTRL1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "CTRL2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1E, "STATUS", 1, RegisterAccess::RO, 0x00),
};

#define SSD1309_REGISTER_COUNT (sizeof(SSD1309_REGISTERS) / sizeof(RegisterDesc))
#endif

SSD1309Driver::SSD1309Driver() : address(0), initialized(false) {}

bool SSD1309Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SSD1309_ENABLE_LOGGING
    Logger::info("SSD1309: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_SSD1309_ENABLE_CONFIGURATION
    writeRegister(SSD1309_REG_CTRL1, 0x00);
    writeRegister(SSD1309_REG_CTRL2, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_SSD1309_ENABLE_LOGGING
    Logger::info("SSD1309: Initialized successfully");
#endif
    return true;
}

void SSD1309Driver::deinit() {
    initialized = false;
}

CapabilitySchema SSD1309Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_SSD1309_TIER_NAME;
    return schema;
}

bool SSD1309Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool SSD1309Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_SSD1309_ENABLE_REGISTER_ACCESS
const RegisterDesc* SSD1309Driver::registers(size_t& count) const {
    count = SSD1309_REGISTER_COUNT;
    return SSD1309_REGISTERS;
}

bool SSD1309Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(SSD1309_REGISTERS, SSD1309_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool SSD1309Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(SSD1309_REGISTERS, SSD1309_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* SSD1309Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(SSD1309_REGISTERS, SSD1309_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
