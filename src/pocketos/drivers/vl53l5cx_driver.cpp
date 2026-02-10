#include "vl53l5cx_driver.h"
#include "../driver_config.h"

#if POCKETOS_VL53L5CX_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VL53L5CX_REG_CONTROL    0x00
#define VL53L5CX_REG_STATUS     0x01

#if POCKETOS_VL53L5CX_ENABLE_REGISTER_ACCESS
static const RegisterDesc VL53L5CX_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
};

#define VL53L5CX_REGISTER_COUNT (sizeof(VL53L5CX_REGISTERS) / sizeof(RegisterDesc))
#endif

VL53L5CXDriver::VL53L5CXDriver() : address(0), initialized(false) {}

bool VL53L5CXDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_VL53L5CX_ENABLE_LOGGING
    Logger::info("VL53L5CX: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_VL53L5CX_ENABLE_CONFIGURATION
    writeRegister(VL53L5CX_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_VL53L5CX_ENABLE_LOGGING
    Logger::info("VL53L5CX: Initialized successfully");
#endif
    return true;
}

void VL53L5CXDriver::deinit() {
    initialized = false;
}

CapabilitySchema VL53L5CXDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_VL53L5CX_TIER_NAME;
    return schema;
}

bool VL53L5CXDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool VL53L5CXDriver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_VL53L5CX_ENABLE_REGISTER_ACCESS
const RegisterDesc* VL53L5CXDriver::registers(size_t& count) const {
    count = VL53L5CX_REGISTER_COUNT;
    return VL53L5CX_REGISTERS;
}

bool VL53L5CXDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VL53L5CX_REGISTERS, VL53L5CX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool VL53L5CXDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VL53L5CX_REGISTERS, VL53L5CX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* VL53L5CXDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(VL53L5CX_REGISTERS, VL53L5CX_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
