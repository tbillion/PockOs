#include "tsl2591_driver.h"
#include "../driver_config.h"

#if POCKETOS_TSL2591_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define TSL2591_CMD           0xA0
#define TSL2591_REG_ENABLE    0x00
#define TSL2591_REG_CONFIG    0x01
#define TSL2591_REG_STATUS    0x13
#define TSL2591_REG_C0DATAL   0x14
#define TSL2591_REG_C1DATAL   0x16
#define TSL2591_REG_ID        0x12

#if POCKETOS_TSL2591_ENABLE_REGISTER_ACCESS
static const RegisterDesc TSL2591_REGISTERS[] = {
    RegisterDesc(0x00, "ENABLE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "CONFIG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "ID", 1, RegisterAccess::RO, 0x50),
    RegisterDesc(0x13, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x14, "C0DATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x15, "C0DATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x16, "C1DATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x17, "C1DATAH", 1, RegisterAccess::RO, 0x00),
};
#define TSL2591_REGISTER_COUNT (sizeof(TSL2591_REGISTERS) / sizeof(RegisterDesc))
#endif

TSL2591Driver::TSL2591Driver() : address(0), initialized(false), gain(0), integrationTime(0) {}

bool TSL2591Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_TSL2591_ENABLE_LOGGING
    Logger::info("TSL2591: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t id;
    if (!readRegister(TSL2591_REG_ID, &id) || id != 0x50) {
#if POCKETOS_TSL2591_ENABLE_LOGGING
        Logger::error("TSL2591: Invalid chip ID");
#endif
        return false;
    }
    
    writeRegister(TSL2591_REG_ENABLE, 0x03);
    writeRegister(TSL2591_REG_CONFIG, 0x00);
    
    initialized = true;
#if POCKETOS_TSL2591_ENABLE_LOGGING
    Logger::info("TSL2591: Initialized successfully");
#endif
    return true;
}

void TSL2591Driver::deinit() {
    if (initialized) writeRegister(TSL2591_REG_ENABLE, 0x00);
    initialized = false;
}

TSL2591Data TSL2591Driver::readData() {
    TSL2591Data data;
    if (!initialized) return data;
    
    delay(120);
    
    uint8_t buffer[4];
    if (!readRegisters(TSL2591_REG_C0DATAL, buffer, 4)) return data;
    
    data.full = (buffer[1] << 8) | buffer[0];
    data.ir = (buffer[3] << 8) | buffer[2];
    data.lux = calculateLux(data.full, data.ir);
    data.valid = true;
    
    return data;
}

float TSL2591Driver::calculateLux(uint16_t full, uint16_t ir) {
    if (full == 0) return 0;
    float atime = 100.0;
    float again = 1.0;
    float cpl = (atime * again) / 408.0;
    float lux1 = ((float)full - ((float)ir * 1.64)) / cpl;
    float lux2 = (((float)full * 0.59) - ((float)ir * 0.86)) / cpl;
    return (lux1 > lux2) ? lux1 : lux2;
}

CapabilitySchema TSL2591Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "tsl2591";
    schema.tier = POCKETOS_TSL2591_TIER_NAME;
    schema.category = "light";
    schema.addOutput("lux", "float", "Ambient light in lux");
    return schema;
}

String TSL2591Driver::getParameter(const String& name) { return ""; }
bool TSL2591Driver::setParameter(const String& name, const String& value) { return false; }

bool TSL2591Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(TSL2591_CMD | reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool TSL2591Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(TSL2591_CMD | reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() != 1) return false;
    *value = Wire.read();
    return true;
}

bool TSL2591Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(TSL2591_CMD | reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)len);
    if (Wire.available() != len) return false;
    for (size_t i = 0; i < len; i++) buffer[i] = Wire.read();
    return true;
}

#if POCKETOS_TSL2591_ENABLE_REGISTER_ACCESS
const RegisterDesc* TSL2591Driver::registers(size_t& count) const {
    count = TSL2591_REGISTER_COUNT;
    return TSL2591_REGISTERS;
}

bool TSL2591Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(TSL2591_REGISTERS, TSL2591_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    return readRegister((uint8_t)reg, buf);
}

bool TSL2591Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(TSL2591_REGISTERS, TSL2591_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* TSL2591Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(TSL2591_REGISTERS, TSL2591_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
