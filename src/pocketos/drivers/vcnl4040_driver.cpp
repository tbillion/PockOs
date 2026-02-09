#include "vcnl4040_driver.h"
#include "../driver_config.h"

#if POCKETOS_VCNL4040_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VCNL4040_REG_ALS_CONF     0x00
#define VCNL4040_REG_PS_CONF      0x03
#define VCNL4040_REG_PS_DATA      0x08
#define VCNL4040_REG_ALS_DATA     0x09
#define VCNL4040_REG_WHITE_DATA   0x0A
#define VCNL4040_REG_ID           0x0C

#if POCKETOS_VCNL4040_ENABLE_REGISTER_ACCESS
static const RegisterDesc VCNL4040_REGISTERS[] = {
    RegisterDesc(0x00, "ALS_CONF", 2, RegisterAccess::RW, 0x0001),
    RegisterDesc(0x03, "PS_CONF", 2, RegisterAccess::RW, 0x0001),
    RegisterDesc(0x08, "PS_DATA", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x09, "ALS_DATA", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x0A, "WHITE_DATA", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x0C, "ID", 2, RegisterAccess::RO, 0x0186),
};
#define VCNL4040_REGISTER_COUNT (sizeof(VCNL4040_REGISTERS) / sizeof(RegisterDesc))
#endif

VCNL4040Driver::VCNL4040Driver() : address(0), initialized(false) {}

bool VCNL4040Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_VCNL4040_ENABLE_LOGGING
    Logger::info("VCNL4040: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint16_t id;
    if (!readRegister(VCNL4040_REG_ID, &id) || id != 0x0186) {
#if POCKETOS_VCNL4040_ENABLE_LOGGING
        Logger::error("VCNL4040: Invalid device ID");
#endif
        return false;
    }
    
    writeRegister(VCNL4040_REG_ALS_CONF, 0x0000);
    writeRegister(VCNL4040_REG_PS_CONF, 0x0000);
    
    initialized = true;
#if POCKETOS_VCNL4040_ENABLE_LOGGING
    Logger::info("VCNL4040: Initialized successfully");
#endif
    return true;
}

void VCNL4040Driver::deinit() {
    if (initialized) {
        writeRegister(VCNL4040_REG_ALS_CONF, 0x0001);
        writeRegister(VCNL4040_REG_PS_CONF, 0x0001);
    }
    initialized = false;
}

VCNL4040Data VCNL4040Driver::readData() {
    VCNL4040Data data;
    if (!initialized) return data;
    
    delay(80);
    
    if (!readRegister(VCNL4040_REG_PS_DATA, &data.proximity)) return data;
    if (!readRegister(VCNL4040_REG_ALS_DATA, &data.white)) return data;
    if (!readRegister(VCNL4040_REG_WHITE_DATA, &data.white)) return data;
    
    data.ambient = data.white * 0.1;
    data.valid = true;
    
    return data;
}

CapabilitySchema VCNL4040Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "vcnl4040";
    schema.tier = POCKETOS_VCNL4040_TIER_NAME;
    schema.category = "proximity";
    schema.addOutput("proximity", "uint16", "Proximity value");
    schema.addOutput("ambient", "float", "Ambient light");
    schema.addOutput("white", "uint16", "White channel");
    return schema;
}

String VCNL4040Driver::getParameter(const String& name) { return ""; }
bool VCNL4040Driver::setParameter(const String& name, const String& value) { return false; }

bool VCNL4040Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value & 0xFF);
    Wire.write((value >> 8) & 0xFF);
    return Wire.endTransmission() == 0;
}

bool VCNL4040Driver::readRegister(uint8_t reg, uint16_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)2);
    if (Wire.available() != 2) return false;
    uint8_t lsb = Wire.read();
    uint8_t msb = Wire.read();
    *value = (msb << 8) | lsb;
    return true;
}

#if POCKETOS_VCNL4040_ENABLE_REGISTER_ACCESS
const RegisterDesc* VCNL4040Driver::registers(size_t& count) const {
    count = VCNL4040_REGISTER_COUNT;
    return VCNL4040_REGISTERS;
}

bool VCNL4040Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VCNL4040_REGISTERS, VCNL4040_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    uint16_t val;
    if (!readRegister((uint8_t)reg, &val)) return false;
    buf[0] = val & 0xFF;
    if (len > 1) buf[1] = (val >> 8) & 0xFF;
    return true;
}

bool VCNL4040Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VCNL4040_REGISTERS, VCNL4040_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    uint16_t val = buf[0];
    if (len > 1) val |= (buf[1] << 8);
    return writeRegister((uint8_t)reg, val);
}

const RegisterDesc* VCNL4040Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(VCNL4040_REGISTERS, VCNL4040_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
