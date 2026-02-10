#include "ism330dhcx_driver.h"
#include "../driver_config.h"

#if POCKETOS_ISM330DHCX_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define ISM330DHCX_REG_WHO_AM_I   0x0F
#define ISM330DHCX_REG_CTRL1      0x10
#define ISM330DHCX_REG_CTRL2      0x11
#define ISM330DHCX_REG_STATUS     0x1E

#if POCKETOS_ISM330DHCX_ENABLE_REGISTER_ACCESS
static const RegisterDesc ISM330DHCX_REGISTERS[] = {
    RegisterDesc(0x0F, "WHO_AM_I", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x10, "CTRL1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "CTRL2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1E, "STATUS", 1, RegisterAccess::RO, 0x00),
};

#define ISM330DHCX_REGISTER_COUNT (sizeof(ISM330DHCX_REGISTERS) / sizeof(RegisterDesc))
#endif

ISM330DHCXDriver::ISM330DHCXDriver() : address(0), initialized(false) {}

bool ISM330DHCXDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_ISM330DHCX_ENABLE_LOGGING
    Logger::info("ISM330DHCX: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_ISM330DHCX_ENABLE_CONFIGURATION
    writeRegister(ISM330DHCX_REG_CTRL1, 0x00);
    writeRegister(ISM330DHCX_REG_CTRL2, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_ISM330DHCX_ENABLE_LOGGING
    Logger::info("ISM330DHCX: Initialized successfully");
#endif
    return true;
}

void ISM330DHCXDriver::deinit() {
    initialized = false;
}

CapabilitySchema ISM330DHCXDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_ISM330DHCX_TIER_NAME;
    return schema;
}

bool ISM330DHCXDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool ISM330DHCXDriver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_ISM330DHCX_ENABLE_REGISTER_ACCESS
const RegisterDesc* ISM330DHCXDriver::registers(size_t& count) const {
    count = ISM330DHCX_REGISTER_COUNT;
    return ISM330DHCX_REGISTERS;
}

bool ISM330DHCXDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(ISM330DHCX_REGISTERS, ISM330DHCX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool ISM330DHCXDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(ISM330DHCX_REGISTERS, ISM330DHCX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* ISM330DHCXDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(ISM330DHCX_REGISTERS, ISM330DHCX_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
