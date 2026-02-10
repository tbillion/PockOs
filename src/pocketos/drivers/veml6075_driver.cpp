#include "veml6075_driver.h"
#include "../driver_config.h"

#if POCKETOS_VEML6075_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VEML6075_REG_CONF        0x00
#define VEML6075_REG_UVA_DATA    0x07
#define VEML6075_REG_UVB_DATA    0x09
#define VEML6075_REG_UVCOMP1     0x0A
#define VEML6075_REG_UVCOMP2     0x0B
#define VEML6075_REG_ID          0x0C

#if POCKETOS_VEML6075_ENABLE_REGISTER_ACCESS
static const RegisterDesc VEML6075_REGISTERS[] = {
    RegisterDesc(0x00, "CONF", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "UVA_DATA", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "UVB_DATA", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "UVCOMP1", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0B, "UVCOMP2", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "ID", 2, RegisterAccess::RO, 0x0026),
};
#define VEML6075_REGISTER_COUNT (sizeof(VEML6075_REGISTERS) / sizeof(RegisterDesc))
#endif

VEML6075Driver::VEML6075Driver() : address(0), initialized(false) {}

bool VEML6075Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_VEML6075_ENABLE_LOGGING
    Logger::info("VEML6075: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint16_t id;
    if (!readRegister(VEML6075_REG_ID, &id) || id != 0x0026) {
#if POCKETOS_VEML6075_ENABLE_LOGGING
        Logger::error("VEML6075: Invalid chip ID");
#endif
        return false;
    }
    
    writeRegister(VEML6075_REG_CONF, 0x00);
    
    initialized = true;
#if POCKETOS_VEML6075_ENABLE_LOGGING
    Logger::info("VEML6075: Initialized successfully");
#endif
    return true;
}

void VEML6075Driver::deinit() {
    if (initialized) writeRegister(VEML6075_REG_CONF, 0x01);
    initialized = false;
}

VEML6075Data VEML6075Driver::readData() {
    VEML6075Data data;
    if (!initialized) return data;
    
    delay(100);
    
    uint16_t uva_raw, uvb_raw, comp1, comp2;
    if (!readRegister(VEML6075_REG_UVA_DATA, &uva_raw)) return data;
    if (!readRegister(VEML6075_REG_UVB_DATA, &uvb_raw)) return data;
    if (!readRegister(VEML6075_REG_UVCOMP1, &comp1)) return data;
    if (!readRegister(VEML6075_REG_UVCOMP2, &comp2)) return data;
    
    float uva_comp = uva_raw - 2.22 * comp1 + 1.33 * comp2;
    float uvb_comp = uvb_raw - 2.95 * comp1 + 1.74 * comp2;
    
    data.uva = uva_comp;
    data.uvb = uvb_comp;
    data.uvIndex = (data.uva + data.uvb) / 2.0;
    data.valid = true;
    
    return data;
}

void VEML6075Driver::calculateUV(uint16_t uva_raw, uint16_t uvb_raw, uint16_t comp1, uint16_t comp2) {}

CapabilitySchema VEML6075Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "veml6075";
    schema.tier = POCKETOS_VEML6075_TIER_NAME;
    schema.category = "uv";
    schema.addOutput("uva", "float", "UVA reading");
    schema.addOutput("uvb", "float", "UVB reading");
    schema.addOutput("uvIndex", "float", "UV index");
    return schema;
}

String VEML6075Driver::getParameter(const String& name) { return ""; }
bool VEML6075Driver::setParameter(const String& name, const String& value) { return false; }

bool VEML6075Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value & 0xFF);
    Wire.write((value >> 8) & 0xFF);
    return Wire.endTransmission() == 0;
}

bool VEML6075Driver::readRegister(uint8_t reg, uint16_t* value) {
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

#if POCKETOS_VEML6075_ENABLE_REGISTER_ACCESS
const RegisterDesc* VEML6075Driver::registers(size_t& count) const {
    count = VEML6075_REGISTER_COUNT;
    return VEML6075_REGISTERS;
}

bool VEML6075Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VEML6075_REGISTERS, VEML6075_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    uint16_t val;
    if (!readRegister((uint8_t)reg, &val)) return false;
    buf[0] = val & 0xFF;
    if (len > 1) buf[1] = (val >> 8) & 0xFF;
    return true;
}

bool VEML6075Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VEML6075_REGISTERS, VEML6075_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    uint16_t val = buf[0];
    if (len > 1) val |= (buf[1] << 8);
    return writeRegister((uint8_t)reg, val);
}

const RegisterDesc* VEML6075Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(VEML6075_REGISTERS, VEML6075_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
