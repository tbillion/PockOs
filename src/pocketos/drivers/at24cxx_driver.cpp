#include "at24cxx_driver.h"
#include "../driver_config.h"

#if POCKETOS_AT24CXX_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define AT24CXX_REG_CONTROL    0x00
#define AT24CXX_REG_STATUS     0x01
#define AT24CXX_REG_DATA       0x02

#if POCKETOS_AT24CXX_ENABLE_REGISTER_ACCESS
static const RegisterDesc AT24CXX_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "DATA", 1, RegisterAccess::RO, 0x00),
};

#define AT24CXX_REGISTER_COUNT (sizeof(AT24CXX_REGISTERS) / sizeof(RegisterDesc))
#endif

AT24CxxDriver::AT24CxxDriver() : address(0), initialized(false) {}

bool AT24CxxDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AT24CXX_ENABLE_LOGGING
    Logger::info("AT24Cxx: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_AT24CXX_ENABLE_CONFIGURATION
    writeRegister(AT24CXX_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_AT24CXX_ENABLE_LOGGING
    Logger::info("AT24Cxx: Initialized successfully");
#endif
    return true;
}

void AT24CxxDriver::deinit() {
    initialized = false;
}

AT24CxxData AT24CxxDriver::readData() {
    AT24CxxData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t value;
    if (readRegister(AT24CXX_REG_DATA, &value)) {
        data.value = value;
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema AT24CxxDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_AT24CXX_TIER_NAME;
    return schema;
}

bool AT24CxxDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool AT24CxxDriver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_AT24CXX_ENABLE_REGISTER_ACCESS
const RegisterDesc* AT24CxxDriver::registers(size_t& count) const {
    count = AT24CXX_REGISTER_COUNT;
    return AT24CXX_REGISTERS;
}

bool AT24CxxDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AT24CXX_REGISTERS, AT24CXX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool AT24CxxDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AT24CXX_REGISTERS, AT24CXX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* AT24CxxDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(AT24CXX_REGISTERS, AT24CXX_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
