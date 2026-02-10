#include "vcnl4010_driver.h"
#include "../driver_config.h"

#if POCKETOS_VCNL4010_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VCNL4010_REG_COMMAND    0x80
#define VCNL4010_REG_PROD_ID    0x81
#define VCNL4010_REG_PROX_RATE  0x82
#define VCNL4010_REG_PROX_DATA  0x87
#define VCNL4010_REG_AMB_DATA   0x85

#if POCKETOS_VCNL4010_ENABLE_REGISTER_ACCESS
static const RegisterDesc VCNL4010_REGISTERS[] = {
    RegisterDesc(0x80, "COMMAND", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x81, "PROD_ID", 1, RegisterAccess::RO, 0x21),
    RegisterDesc(0x82, "PROX_RATE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x85, "AMB_DATA", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x87, "PROX_DATA", 2, RegisterAccess::RO, 0x00),
};
#define VCNL4010_REGISTER_COUNT (sizeof(VCNL4010_REGISTERS) / sizeof(RegisterDesc))
#endif

VCNL4010Driver::VCNL4010Driver() : address(0), initialized(false) {}

bool VCNL4010Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_VCNL4010_ENABLE_LOGGING
    Logger::info("VCNL4010: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t id;
    if (!readRegister(VCNL4010_REG_PROD_ID, &id) || id != 0x21) {
#if POCKETOS_VCNL4010_ENABLE_LOGGING
        Logger::error("VCNL4010: Invalid product ID");
#endif
        return false;
    }
    
    writeRegister(VCNL4010_REG_PROX_RATE, 0x00);
    writeRegister(VCNL4010_REG_COMMAND, 0x18);
    
    initialized = true;
#if POCKETOS_VCNL4010_ENABLE_LOGGING
    Logger::info("VCNL4010: Initialized successfully");
#endif
    return true;
}

void VCNL4010Driver::deinit() {
    initialized = false;
}

VCNL4010Data VCNL4010Driver::readData() {
    VCNL4010Data data;
    if (!initialized) return data;
    
    uint8_t buffer[4];
    if (!readRegisters(VCNL4010_REG_AMB_DATA, buffer, 4)) return data;
    
    data.ambient = (buffer[0] << 8) | buffer[1];
    data.proximity = (buffer[2] << 8) | buffer[3];
    data.valid = true;
    
    return data;
}

CapabilitySchema VCNL4010Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "vcnl4010";
    schema.tier = POCKETOS_VCNL4010_TIER_NAME;
    schema.category = "proximity";
    schema.addOutput("proximity", "uint16", "Proximity value");
    schema.addOutput("ambient", "float", "Ambient light");
    return schema;
}

String VCNL4010Driver::getParameter(const String& name) { return ""; }
bool VCNL4010Driver::setParameter(const String& name, const String& value) { return false; }

bool VCNL4010Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool VCNL4010Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() != 1) return false;
    *value = Wire.read();
    return true;
}

bool VCNL4010Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)len);
    if (Wire.available() != len) return false;
    for (size_t i = 0; i < len; i++) buffer[i] = Wire.read();
    return true;
}

#if POCKETOS_VCNL4010_ENABLE_REGISTER_ACCESS
const RegisterDesc* VCNL4010Driver::registers(size_t& count) const {
    count = VCNL4010_REGISTER_COUNT;
    return VCNL4010_REGISTERS;
}

bool VCNL4010Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VCNL4010_REGISTERS, VCNL4010_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    return readRegister((uint8_t)reg, buf);
}

bool VCNL4010Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VCNL4010_REGISTERS, VCNL4010_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* VCNL4010Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(VCNL4010_REGISTERS, VCNL4010_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
