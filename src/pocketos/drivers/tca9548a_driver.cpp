#include "tca9548a_driver.h"
#include "../driver_config.h"

#if POCKETOS_TCA9548A_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define TCA9548A_REG_CONTROL    0x00
#define TCA9548A_REG_STATUS     0x01

#if POCKETOS_TCA9548A_ENABLE_REGISTER_ACCESS
static const RegisterDesc TCA9548A_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
};

#define TCA9548A_REGISTER_COUNT (sizeof(TCA9548A_REGISTERS) / sizeof(RegisterDesc))
#endif

TCA9548ADriver::TCA9548ADriver() : address(0), initialized(false) {}

bool TCA9548ADriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_TCA9548A_ENABLE_LOGGING
    Logger::info("TCA9548A: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_TCA9548A_ENABLE_CONFIGURATION
    writeRegister(TCA9548A_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_TCA9548A_ENABLE_LOGGING
    Logger::info("TCA9548A: Initialized successfully");
#endif
    return true;
}

void TCA9548ADriver::deinit() {
    initialized = false;
}

CapabilitySchema TCA9548ADriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_TCA9548A_TIER_NAME;
    return schema;
}

bool TCA9548ADriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool TCA9548ADriver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_TCA9548A_ENABLE_REGISTER_ACCESS
const RegisterDesc* TCA9548ADriver::registers(size_t& count) const {
    count = TCA9548A_REGISTER_COUNT;
    return TCA9548A_REGISTERS;
}

bool TCA9548ADriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(TCA9548A_REGISTERS, TCA9548A_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool TCA9548ADriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(TCA9548A_REGISTERS, TCA9548A_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* TCA9548ADriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(TCA9548A_REGISTERS, TCA9548A_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
