#include "vl6180x_driver.h"
#include "../driver_config.h"

#if POCKETOS_VL6180X_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VL6180X_REG_CONTROL    0x00
#define VL6180X_REG_STATUS     0x01

#if POCKETOS_VL6180X_ENABLE_REGISTER_ACCESS
static const RegisterDesc VL6180X_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
};

#define VL6180X_REGISTER_COUNT (sizeof(VL6180X_REGISTERS) / sizeof(RegisterDesc))
#endif

VL6180XDriver::VL6180XDriver() : address(0), initialized(false) {}

bool VL6180XDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_VL6180X_ENABLE_LOGGING
    Logger::info("VL6180X: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_VL6180X_ENABLE_CONFIGURATION
    writeRegister(VL6180X_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_VL6180X_ENABLE_LOGGING
    Logger::info("VL6180X: Initialized successfully");
#endif
    return true;
}

void VL6180XDriver::deinit() {
    initialized = false;
}

CapabilitySchema VL6180XDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_VL6180X_TIER_NAME;
    return schema;
}

bool VL6180XDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool VL6180XDriver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_VL6180X_ENABLE_REGISTER_ACCESS
const RegisterDesc* VL6180XDriver::registers(size_t& count) const {
    count = VL6180X_REGISTER_COUNT;
    return VL6180X_REGISTERS;
}

bool VL6180XDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VL6180X_REGISTERS, VL6180X_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool VL6180XDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VL6180X_REGISTERS, VL6180X_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* VL6180XDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(VL6180X_REGISTERS, VL6180X_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
