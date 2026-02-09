#include "vl53l1x_driver.h"
#include "../driver_config.h"

#if POCKETOS_VL53L1X_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VL53L1X_REG_CONTROL    0x00
#define VL53L1X_REG_STATUS     0x01

#if POCKETOS_VL53L1X_ENABLE_REGISTER_ACCESS
static const RegisterDesc VL53L1X_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
};

#define VL53L1X_REGISTER_COUNT (sizeof(VL53L1X_REGISTERS) / sizeof(RegisterDesc))
#endif

VL53L1XDriver::VL53L1XDriver() : address(0), initialized(false) {}

bool VL53L1XDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_VL53L1X_ENABLE_LOGGING
    Logger::info("VL53L1X: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_VL53L1X_ENABLE_CONFIGURATION
    writeRegister(VL53L1X_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_VL53L1X_ENABLE_LOGGING
    Logger::info("VL53L1X: Initialized successfully");
#endif
    return true;
}

void VL53L1XDriver::deinit() {
    initialized = false;
}

CapabilitySchema VL53L1XDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_VL53L1X_TIER_NAME;
    return schema;
}

bool VL53L1XDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool VL53L1XDriver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_VL53L1X_ENABLE_REGISTER_ACCESS
const RegisterDesc* VL53L1XDriver::registers(size_t& count) const {
    count = VL53L1X_REGISTER_COUNT;
    return VL53L1X_REGISTERS;
}

bool VL53L1XDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VL53L1X_REGISTERS, VL53L1X_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool VL53L1XDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VL53L1X_REGISTERS, VL53L1X_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* VL53L1XDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(VL53L1X_REGISTERS, VL53L1X_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
