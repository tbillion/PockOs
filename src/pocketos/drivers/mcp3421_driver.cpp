#include "mcp3421_driver.h"
#include "../driver_config.h"

#if POCKETOS_MCP3421_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define MCP3421_REG_CONTROL    0x00
#define MCP3421_REG_STATUS     0x01
#define MCP3421_REG_DATA       0x02

#if POCKETOS_MCP3421_ENABLE_REGISTER_ACCESS
static const RegisterDesc MCP3421_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "DATA", 1, RegisterAccess::RO, 0x00),
};

#define MCP3421_REGISTER_COUNT (sizeof(MCP3421_REGISTERS) / sizeof(RegisterDesc))
#endif

MCP3421Driver::MCP3421Driver() : address(0), initialized(false) {}

bool MCP3421Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MCP3421_ENABLE_LOGGING
    Logger::info("MCP3421: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_MCP3421_ENABLE_CONFIGURATION
    writeRegister(MCP3421_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_MCP3421_ENABLE_LOGGING
    Logger::info("MCP3421: Initialized successfully");
#endif
    return true;
}

void MCP3421Driver::deinit() {
    initialized = false;
}

MCP3421Data MCP3421Driver::readData() {
    MCP3421Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t value;
    if (readRegister(MCP3421_REG_DATA, &value)) {
        data.value = value;
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema MCP3421Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_MCP3421_TIER_NAME;
    return schema;
}

bool MCP3421Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MCP3421Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_MCP3421_ENABLE_REGISTER_ACCESS
const RegisterDesc* MCP3421Driver::registers(size_t& count) const {
    count = MCP3421_REGISTER_COUNT;
    return MCP3421_REGISTERS;
}

bool MCP3421Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MCP3421_REGISTERS, MCP3421_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool MCP3421Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MCP3421_REGISTERS, MCP3421_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MCP3421Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MCP3421_REGISTERS, MCP3421_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
