#include "mcp4725_driver.h"
#include "../driver_config.h"

#if POCKETOS_MCP4725_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define MCP4725_REG_CONTROL    0x00
#define MCP4725_REG_STATUS     0x01
#define MCP4725_REG_DATA       0x02

#if POCKETOS_MCP4725_ENABLE_REGISTER_ACCESS
static const RegisterDesc MCP4725_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "DATA", 1, RegisterAccess::RO, 0x00),
};

#define MCP4725_REGISTER_COUNT (sizeof(MCP4725_REGISTERS) / sizeof(RegisterDesc))
#endif

MCP4725Driver::MCP4725Driver() : address(0), initialized(false) {}

bool MCP4725Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MCP4725_ENABLE_LOGGING
    Logger::info("MCP4725: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_MCP4725_ENABLE_CONFIGURATION
    writeRegister(MCP4725_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_MCP4725_ENABLE_LOGGING
    Logger::info("MCP4725: Initialized successfully");
#endif
    return true;
}

void MCP4725Driver::deinit() {
    initialized = false;
}

MCP4725Data MCP4725Driver::readData() {
    MCP4725Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t value;
    if (readRegister(MCP4725_REG_DATA, &value)) {
        data.value = value;
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema MCP4725Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_MCP4725_TIER_NAME;
    return schema;
}

bool MCP4725Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MCP4725Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_MCP4725_ENABLE_REGISTER_ACCESS
const RegisterDesc* MCP4725Driver::registers(size_t& count) const {
    count = MCP4725_REGISTER_COUNT;
    return MCP4725_REGISTERS;
}

bool MCP4725Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MCP4725_REGISTERS, MCP4725_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool MCP4725Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MCP4725_REGISTERS, MCP4725_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MCP4725Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MCP4725_REGISTERS, MCP4725_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
