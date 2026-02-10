#include "as5600_driver.h"
#include "../driver_config.h"

#if POCKETOS_AS5600_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define AS5600_REG_RAW_ANGLE_H    0x0C
#define AS5600_REG_RAW_ANGLE_L    0x0D
#define AS5600_REG_ANGLE_H        0x0E
#define AS5600_REG_ANGLE_L        0x0F
#define AS5600_REG_STATUS         0x0B
#define AS5600_REG_AGC            0x1A
#define AS5600_REG_MAGNITUDE_H    0x1B
#define AS5600_REG_MAGNITUDE_L    0x1C

AS5600Driver::AS5600Driver() : address(0), initialized(false) {}

bool AS5600Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AS5600_ENABLE_LOGGING
    Logger::info("AS5600: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t status = 0;
    if (!readRegister(AS5600_REG_STATUS, &status)) {
#if POCKETOS_AS5600_ENABLE_LOGGING
        Logger::error("AS5600: Failed to read status");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_AS5600_ENABLE_LOGGING
    Logger::info("AS5600: Initialized successfully");
#endif
    return true;
}

void AS5600Driver::deinit() {
    initialized = false;
}

AS5600Data AS5600Driver::readData() {
    AS5600Data data;
    
    if (!initialized) {
        return data;
    }
    
    if (!readRegister(AS5600_REG_STATUS, &data.status)) {
        return data;
    }
    
    if (!readWord(AS5600_REG_ANGLE_H, &data.angle)) {
        return data;
    }
    
    if (!readWord(AS5600_REG_RAW_ANGLE_H, &data.raw_angle)) {
        return data;
    }
    
    data.angle &= 0x0FFF;
    data.raw_angle &= 0x0FFF;
    data.valid = true;
    
    return data;
}

CapabilitySchema AS5600Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "as5600";
    schema.tier = POCKETOS_AS5600_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("angle", "Filtered angle", "12-bit", "0-4095"));
    schema.outputs.push_back(OutputDesc("raw_angle", "Raw angle", "12-bit", "0-4095"));
    schema.outputs.push_back(OutputDesc("status", "Status flags", "bitmask", "0-255"));
    
    return schema;
}

String AS5600Driver::getParameter(const String& name) {
    return "";
}

bool AS5600Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool AS5600Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
        return false;
    }
    
    *value = Wire.read();
    return true;
}

bool AS5600Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool AS5600Driver::readWord(uint8_t reg, uint16_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)2) != 2) {
        return false;
    }
    
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    *value = (msb << 8) | lsb;
    return true;
}

#if POCKETOS_AS5600_ENABLE_REGISTER_ACCESS
static const RegisterDesc AS5600_REGISTERS[] = {
    RegisterDesc(0x0B, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "RAW_ANGLE_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "RAW_ANGLE_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0E, "ANGLE_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0F, "ANGLE_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1A, "AGC", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1B, "MAGNITUDE_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1C, "MAGNITUDE_L", 1, RegisterAccess::RO, 0x00),
};

#define AS5600_REGISTER_COUNT (sizeof(AS5600_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* AS5600Driver::registers(size_t& count) const {
    count = AS5600_REGISTER_COUNT;
    return AS5600_REGISTERS;
}

bool AS5600Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf);
}

bool AS5600Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* AS5600Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < AS5600_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(AS5600_REGISTERS[i].name)) {
            return &AS5600_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
