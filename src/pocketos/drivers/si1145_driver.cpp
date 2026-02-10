#include "si1145_driver.h"
#include "../driver_config.h"

#if POCKETOS_SI1145_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define SI1145_REG_PART_ID     0x00
#define SI1145_REG_HW_KEY      0x07
#define SI1145_REG_COMMAND     0x18
#define SI1145_REG_RESPONSE    0x2E
#define SI1145_REG_ALS_VIS     0x22
#define SI1145_REG_ALS_IR      0x24
#define SI1145_REG_AUX_DATA    0x2C

#if POCKETOS_SI1145_ENABLE_REGISTER_ACCESS
static const RegisterDesc SI1145_REGISTERS[] = {
    RegisterDesc(0x00, "PART_ID", 1, RegisterAccess::RO, 0x45),
    RegisterDesc(0x07, "HW_KEY", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x18, "COMMAND", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x22, "ALS_VIS", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x24, "ALS_IR", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "AUX_DATA", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2E, "RESPONSE", 1, RegisterAccess::RO, 0x00),
};
#define SI1145_REGISTER_COUNT (sizeof(SI1145_REGISTERS) / sizeof(RegisterDesc))
#endif

SI1145Driver::SI1145Driver() : address(0), initialized(false) {}

bool SI1145Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SI1145_ENABLE_LOGGING
    Logger::info("SI1145: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t id;
    if (!readRegister(SI1145_REG_PART_ID, &id) || id != 0x45) {
#if POCKETOS_SI1145_ENABLE_LOGGING
        Logger::error("SI1145: Invalid part ID");
#endif
        return false;
    }
    
    if (!reset()) return false;
    
    writeParam(0x01, 0x80 | 0x20 | 0x10);
    writeRegister(SI1145_REG_COMMAND, 0x0F);
    
    initialized = true;
#if POCKETOS_SI1145_ENABLE_LOGGING
    Logger::info("SI1145: Initialized successfully");
#endif
    return true;
}

void SI1145Driver::deinit() {
    initialized = false;
}

bool SI1145Driver::reset() {
    writeRegister(SI1145_REG_COMMAND, 0x01);
    delay(10);
    writeRegister(SI1145_REG_HW_KEY, 0x17);
    delay(10);
    return true;
}

SI1145Data SI1145Driver::readData() {
    SI1145Data data;
    if (!initialized) return data;
    
    uint8_t buffer[6];
    if (!readRegisters(SI1145_REG_ALS_VIS, buffer, 6)) return data;
    
    data.visible = (buffer[1] << 8) | buffer[0];
    data.ir = (buffer[3] << 8) | buffer[2];
    data.uv = (buffer[5] << 8) | buffer[4];
    data.uvIndex = data.uv / 100.0;
    data.valid = true;
    
    return data;
}

CapabilitySchema SI1145Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "si1145";
    schema.tier = POCKETOS_SI1145_TIER_NAME;
    schema.category = "light";
    schema.addOutput("visible", "float", "Visible light");
    schema.addOutput("ir", "float", "Infrared");
    schema.addOutput("uv", "float", "UV");
    schema.addOutput("uvIndex", "float", "UV index");
    return schema;
}

String SI1145Driver::getParameter(const String& name) { return ""; }
bool SI1145Driver::setParameter(const String& name, const String& value) { return false; }

bool SI1145Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool SI1145Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() != 1) return false;
    *value = Wire.read();
    return true;
}

bool SI1145Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)len);
    if (Wire.available() != len) return false;
    for (size_t i = 0; i < len; i++) buffer[i] = Wire.read();
    return true;
}

bool SI1145Driver::writeParam(uint8_t param, uint8_t value) {
    writeRegister(0x17, value);
    writeRegister(SI1145_REG_COMMAND, 0xA0 | param);
    delay(10);
    return true;
}

#if POCKETOS_SI1145_ENABLE_REGISTER_ACCESS
const RegisterDesc* SI1145Driver::registers(size_t& count) const {
    count = SI1145_REGISTER_COUNT;
    return SI1145_REGISTERS;
}

bool SI1145Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(SI1145_REGISTERS, SI1145_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    return readRegister((uint8_t)reg, buf);
}

bool SI1145Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(SI1145_REGISTERS, SI1145_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* SI1145Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(SI1145_REGISTERS, SI1145_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
