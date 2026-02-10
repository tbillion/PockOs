#include "pn532_driver.h"
#include "../driver_config.h"

#if POCKETOS_PN532_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define PN532_REG_CONTROL    0x00
#define PN532_REG_STATUS     0x01
#define PN532_REG_CONFIG     0x02

#if POCKETOS_PN532_ENABLE_REGISTER_ACCESS
static const RegisterDesc PN532_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "CONFIG", 1, RegisterAccess::RW, 0x00),
};

#define PN532_REGISTER_COUNT (sizeof(PN532_REGISTERS) / sizeof(RegisterDesc))
#endif

PN532Driver::PN532Driver() : address(0), initialized(false) {}

bool PN532Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_PN532_ENABLE_LOGGING
    Logger::info("PN532: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_PN532_ENABLE_CONFIGURATION
    writeRegister(PN532_REG_CONTROL, 0x00);
    writeRegister(PN532_REG_CONFIG, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_PN532_ENABLE_LOGGING
    Logger::info("PN532: Initialized successfully");
#endif
    return true;
}

void PN532Driver::deinit() {
    initialized = false;
}

PN532Data PN532Driver::readData() {
    PN532Data data;
    
    if (!initialized) {
        return data;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema PN532Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_PN532_TIER_NAME;
    return schema;
}

bool PN532Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool PN532Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_PN532_ENABLE_REGISTER_ACCESS
const RegisterDesc* PN532Driver::registers(size_t& count) const {
    count = PN532_REGISTER_COUNT;
    return PN532_REGISTERS;
}

bool PN532Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(PN532_REGISTERS, PN532_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool PN532Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(PN532_REGISTERS, PN532_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* PN532Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PN532_REGISTERS, PN532_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
