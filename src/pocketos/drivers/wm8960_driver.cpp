#include "wm8960_driver.h"
#include "../driver_config.h"

#if POCKETOS_WM8960_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define WM8960_REG_CONTROL    0x00
#define WM8960_REG_STATUS     0x01
#define WM8960_REG_CONFIG     0x02

#if POCKETOS_WM8960_ENABLE_REGISTER_ACCESS
static const RegisterDesc WM8960_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "CONFIG", 1, RegisterAccess::RW, 0x00),
};

#define WM8960_REGISTER_COUNT (sizeof(WM8960_REGISTERS) / sizeof(RegisterDesc))
#endif

WM8960Driver::WM8960Driver() : address(0), initialized(false) {}

bool WM8960Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_WM8960_ENABLE_LOGGING
    Logger::info("WM8960: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_WM8960_ENABLE_CONFIGURATION
    writeRegister(WM8960_REG_CONTROL, 0x00);
    writeRegister(WM8960_REG_CONFIG, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_WM8960_ENABLE_LOGGING
    Logger::info("WM8960: Initialized successfully");
#endif
    return true;
}

void WM8960Driver::deinit() {
    initialized = false;
}

WM8960Data WM8960Driver::readData() {
    WM8960Data data;
    
    if (!initialized) {
        return data;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema WM8960Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_WM8960_TIER_NAME;
    return schema;
}

bool WM8960Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool WM8960Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_WM8960_ENABLE_REGISTER_ACCESS
const RegisterDesc* WM8960Driver::registers(size_t& count) const {
    count = WM8960_REGISTER_COUNT;
    return WM8960_REGISTERS;
}

bool WM8960Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(WM8960_REGISTERS, WM8960_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool WM8960Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(WM8960_REGISTERS, WM8960_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* WM8960Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(WM8960_REGISTERS, WM8960_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
