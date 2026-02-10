#include "sc16is750_driver.h"
#include "../driver_config.h"

#if POCKETOS_SC16IS750_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define SC16IS750_REG_CONTROL    0x00
#define SC16IS750_REG_STATUS     0x01
#define SC16IS750_REG_CONFIG     0x02

#if POCKETOS_SC16IS750_ENABLE_REGISTER_ACCESS
static const RegisterDesc SC16IS750_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "CONFIG", 1, RegisterAccess::RW, 0x00),
};

#define SC16IS750_REGISTER_COUNT (sizeof(SC16IS750_REGISTERS) / sizeof(RegisterDesc))
#endif

SC16IS750Driver::SC16IS750Driver() : address(0), initialized(false) {}

bool SC16IS750Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SC16IS750_ENABLE_LOGGING
    Logger::info("SC16IS750: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_SC16IS750_ENABLE_CONFIGURATION
    writeRegister(SC16IS750_REG_CONTROL, 0x00);
    writeRegister(SC16IS750_REG_CONFIG, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_SC16IS750_ENABLE_LOGGING
    Logger::info("SC16IS750: Initialized successfully");
#endif
    return true;
}

void SC16IS750Driver::deinit() {
    initialized = false;
}

SC16IS750Data SC16IS750Driver::readData() {
    SC16IS750Data data;
    
    if (!initialized) {
        return data;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema SC16IS750Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_SC16IS750_TIER_NAME;
    return schema;
}

bool SC16IS750Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool SC16IS750Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_SC16IS750_ENABLE_REGISTER_ACCESS
const RegisterDesc* SC16IS750Driver::registers(size_t& count) const {
    count = SC16IS750_REGISTER_COUNT;
    return SC16IS750_REGISTERS;
}

bool SC16IS750Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(SC16IS750_REGISTERS, SC16IS750_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool SC16IS750Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(SC16IS750_REGISTERS, SC16IS750_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* SC16IS750Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(SC16IS750_REGISTERS, SC16IS750_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
