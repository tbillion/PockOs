#include "mcp4728_driver.h"
#include "../driver_config.h"

#if POCKETOS_MCP4728_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define MCP4728_REG_CONTROL    0x00
#define MCP4728_REG_STATUS     0x01
#define MCP4728_REG_DATA       0x02

#if POCKETOS_MCP4728_ENABLE_REGISTER_ACCESS
static const RegisterDesc MCP4728_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "DATA", 1, RegisterAccess::RO, 0x00),
};

#define MCP4728_REGISTER_COUNT (sizeof(MCP4728_REGISTERS) / sizeof(RegisterDesc))
#endif

MCP4728Driver::MCP4728Driver() : address(0), initialized(false) {}

bool MCP4728Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MCP4728_ENABLE_LOGGING
    Logger::info("MCP4728: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_MCP4728_ENABLE_CONFIGURATION
    writeRegister(MCP4728_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_MCP4728_ENABLE_LOGGING
    Logger::info("MCP4728: Initialized successfully");
#endif
    return true;
}

void MCP4728Driver::deinit() {
    initialized = false;
}

MCP4728Data MCP4728Driver::readData() {
    MCP4728Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t value;
    if (readRegister(MCP4728_REG_DATA, &value)) {
        data.value = value;
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema MCP4728Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_MCP4728_TIER_NAME;
    return schema;
}

bool MCP4728Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MCP4728Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_MCP4728_ENABLE_REGISTER_ACCESS
const RegisterDesc* MCP4728Driver::registers(size_t& count) const {
    count = MCP4728_REGISTER_COUNT;
    return MCP4728_REGISTERS;
}

bool MCP4728Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MCP4728_REGISTERS, MCP4728_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool MCP4728Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MCP4728_REGISTERS, MCP4728_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MCP4728Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MCP4728_REGISTERS, MCP4728_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
