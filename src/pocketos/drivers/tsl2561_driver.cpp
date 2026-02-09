#include "tsl2561_driver.h"
#include "../driver_config.h"

#if POCKETOS_TSL2561_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define TSL2561_CMD           0x80
#define TSL2561_CMD_WORD      0xA0
#define TSL2561_REG_CONTROL   0x00
#define TSL2561_REG_TIMING    0x01
#define TSL2561_REG_DATA0LOW  0x0C
#define TSL2561_REG_DATA1LOW  0x0E
#define TSL2561_REG_ID        0x0A

#if POCKETOS_TSL2561_ENABLE_REGISTER_ACCESS
static const RegisterDesc TSL2561_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "TIMING", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "ID", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "DATA0LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "DATA0HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0E, "DATA1LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0F, "DATA1HIGH", 1, RegisterAccess::RO, 0x00),
};
#define TSL2561_REGISTER_COUNT (sizeof(TSL2561_REGISTERS) / sizeof(RegisterDesc))
#endif

TSL2561Driver::TSL2561Driver() : address(0), initialized(false), gain(0), integrationTime(0) {}

bool TSL2561Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_TSL2561_ENABLE_LOGGING
    Logger::info("TSL2561: Initializing at address 0x" + String(address, HEX));
#endif
    
    writeRegister(TSL2561_REG_CONTROL, 0x03);
    delay(10);
    
#if POCKETOS_TSL2561_ENABLE_CONFIGURATION
    gain = 0;
    integrationTime = 0x02;
    writeRegister(TSL2561_REG_TIMING, integrationTime);
#else
    writeRegister(TSL2561_REG_TIMING, 0x02);
#endif
    
    initialized = true;
#if POCKETOS_TSL2561_ENABLE_LOGGING
    Logger::info("TSL2561: Initialized successfully");
#endif
    return true;
}

void TSL2561Driver::deinit() {
    if (initialized) {
        writeRegister(TSL2561_REG_CONTROL, 0x00);
    }
    initialized = false;
}

TSL2561Data TSL2561Driver::readData() {
    TSL2561Data data;
    
    if (!initialized) return data;
    
    delay(402);
    
    uint8_t buffer[4];
    if (!readRegisters(TSL2561_REG_DATA0LOW, buffer, 4)) return data;
    
    data.broadband = (buffer[1] << 8) | buffer[0];
    data.ir = (buffer[3] << 8) | buffer[2];
    data.lux = calculateLux(data.broadband, data.ir);
    data.valid = true;
    
    return data;
}

float TSL2561Driver::calculateLux(uint16_t broadband, uint16_t ir) {
    if (broadband == 0) return 0;
    
    float ratio = (float)ir / (float)broadband;
    float lux = 0;
    
    if (ratio <= 0.50) {
        lux = 0.0304 * broadband - 0.062 * broadband * pow(ratio, 1.4);
    } else if (ratio <= 0.61) {
        lux = 0.0224 * broadband - 0.031 * ir;
    } else if (ratio <= 0.80) {
        lux = 0.0128 * broadband - 0.0153 * ir;
    } else if (ratio <= 1.30) {
        lux = 0.00146 * broadband - 0.00112 * ir;
    }
    
    return lux;
}

CapabilitySchema TSL2561Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "tsl2561";
    schema.tier = POCKETOS_TSL2561_TIER_NAME;
    schema.category = "light";
    schema.addOutput("lux", "float", "Ambient light in lux");
    return schema;
}

String TSL2561Driver::getParameter(const String& name) { return ""; }
bool TSL2561Driver::setParameter(const String& name, const String& value) { return false; }

bool TSL2561Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(TSL2561_CMD | reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool TSL2561Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(TSL2561_CMD | reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() != 1) return false;
    *value = Wire.read();
    return true;
}

bool TSL2561Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(TSL2561_CMD_WORD | reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)len);
    if (Wire.available() != len) return false;
    for (size_t i = 0; i < len; i++) buffer[i] = Wire.read();
    return true;
}

#if POCKETOS_TSL2561_ENABLE_REGISTER_ACCESS
const RegisterDesc* TSL2561Driver::registers(size_t& count) const {
    count = TSL2561_REGISTER_COUNT;
    return TSL2561_REGISTERS;
}

bool TSL2561Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(TSL2561_REGISTERS, TSL2561_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    return readRegister((uint8_t)reg, buf);
}

bool TSL2561Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(TSL2561_REGISTERS, TSL2561_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* TSL2561Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(TSL2561_REGISTERS, TSL2561_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
