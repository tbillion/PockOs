#include "veml7700_driver.h"
#include "../driver_config.h"

#if POCKETOS_VEML7700_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VEML7700_REG_CONF       0x00
#define VEML7700_REG_ALS        0x04
#define VEML7700_REG_WHITE      0x05

#if POCKETOS_VEML7700_ENABLE_REGISTER_ACCESS
static const RegisterDesc VEML7700_REGISTERS[] = {
    RegisterDesc(0x00, "CONF", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x04, "ALS", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x05, "WHITE", 2, RegisterAccess::RO, 0x0000),
};
#define VEML7700_REGISTER_COUNT (sizeof(VEML7700_REGISTERS) / sizeof(RegisterDesc))
#endif

VEML7700Driver::VEML7700Driver() : address(0), initialized(false), gain(0), integrationTime(0) {}

bool VEML7700Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_VEML7700_ENABLE_LOGGING
    Logger::info("VEML7700: Initializing at address 0x" + String(address, HEX));
#endif
    
    writeRegister(VEML7700_REG_CONF, 0x0000);
    
    initialized = true;
#if POCKETOS_VEML7700_ENABLE_LOGGING
    Logger::info("VEML7700: Initialized successfully");
#endif
    return true;
}

void VEML7700Driver::deinit() {
    if (initialized) writeRegister(VEML7700_REG_CONF, 0x0001);
    initialized = false;
}

VEML7700Data VEML7700Driver::readData() {
    VEML7700Data data;
    if (!initialized) return data;
    
    delay(100);
    
    uint16_t als, white;
    if (!readRegister(VEML7700_REG_ALS, &als)) return data;
    if (!readRegister(VEML7700_REG_WHITE, &white)) return data;
    
    data.als = als;
    data.white = white;
    data.lux = calculateLux(als);
    data.valid = true;
    
    return data;
}

float VEML7700Driver::calculateLux(uint16_t als) {
    return als * 0.0576;
}

CapabilitySchema VEML7700Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "veml7700";
    schema.tier = POCKETOS_VEML7700_TIER_NAME;
    schema.category = "light";
    schema.addOutput("lux", "float", "Ambient light in lux");
    schema.addOutput("white", "float", "White channel");
    return schema;
}

String VEML7700Driver::getParameter(const String& name) { return ""; }
bool VEML7700Driver::setParameter(const String& name, const String& value) { return false; }

bool VEML7700Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value & 0xFF);
    Wire.write((value >> 8) & 0xFF);
    return Wire.endTransmission() == 0;
}

bool VEML7700Driver::readRegister(uint8_t reg, uint16_t* value) {
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

#if POCKETOS_VEML7700_ENABLE_REGISTER_ACCESS
const RegisterDesc* VEML7700Driver::registers(size_t& count) const {
    count = VEML7700_REGISTER_COUNT;
    return VEML7700_REGISTERS;
}

bool VEML7700Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VEML7700_REGISTERS, VEML7700_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    uint16_t val;
    if (!readRegister((uint8_t)reg, &val)) return false;
    buf[0] = val & 0xFF;
    if (len > 1) buf[1] = (val >> 8) & 0xFF;
    return true;
}

bool VEML7700Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(VEML7700_REGISTERS, VEML7700_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    uint16_t val = buf[0];
    if (len > 1) val |= (buf[1] << 8);
    return writeRegister((uint8_t)reg, val);
}

const RegisterDesc* VEML7700Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(VEML7700_REGISTERS, VEML7700_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
