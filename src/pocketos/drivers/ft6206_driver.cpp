#include "ft6206_driver.h"
#include "../driver_config.h"

#if POCKETOS_FT6206_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define FT6206_REG_MODE           0x00
#define FT6206_REG_TD_STATUS      0x02
#define FT6206_REG_P1_XH          0x03
#define FT6206_REG_CHIPID         0xA3
#define FT6206_REG_FIRMID         0xA6

#define FT6206_CHIP_ID            0x06

FT6206Driver::FT6206Driver() : address(0), initialized(false) {}

bool FT6206Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_FT6206_ENABLE_LOGGING
    Logger::info("FT6206: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t chipId = 0;
    if (!readRegister(FT6206_REG_CHIPID, &chipId)) {
#if POCKETOS_FT6206_ENABLE_LOGGING
        Logger::error("FT6206: Failed to read chip ID");
#endif
        return false;
    }
    
    if (chipId != FT6206_CHIP_ID && chipId != 0x64) {
#if POCKETOS_FT6206_ENABLE_LOGGING
        Logger::error("FT6206: Invalid chip ID: 0x" + String(chipId, HEX));
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_FT6206_ENABLE_LOGGING
    Logger::info("FT6206: Initialized successfully");
#endif
    return true;
}

void FT6206Driver::deinit() {
    initialized = false;
}

FT6206Data FT6206Driver::readData() {
    FT6206Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[16];
    if (!readBlock(FT6206_REG_TD_STATUS, buffer, 16)) {
        return data;
    }
    
    data.touches = buffer[0] & 0x0F;
    
    if (data.touches > 0) {
        data.point1.x = ((buffer[1] & 0x0F) << 8) | buffer[2];
        data.point1.y = ((buffer[3] & 0x0F) << 8) | buffer[4];
        data.point1.event = (buffer[1] >> 6) & 0x03;
        data.point1.valid = true;
    }
    
    if (data.touches > 1) {
        data.point2.x = ((buffer[7] & 0x0F) << 8) | buffer[8];
        data.point2.y = ((buffer[9] & 0x0F) << 8) | buffer[10];
        data.point2.event = (buffer[7] >> 6) & 0x03;
        data.point2.valid = true;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema FT6206Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ft6206";
    schema.tier = POCKETOS_FT6206_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("touches", "Touch count", "count", "0-2"));
    schema.outputs.push_back(OutputDesc("x1", "Point 1 X", "px", "0-4095"));
    schema.outputs.push_back(OutputDesc("y1", "Point 1 Y", "px", "0-4095"));
    schema.outputs.push_back(OutputDesc("x2", "Point 2 X", "px", "0-4095"));
    schema.outputs.push_back(OutputDesc("y2", "Point 2 Y", "px", "0-4095"));
    
    return schema;
}

String FT6206Driver::getParameter(const String& name) {
    return "";
}

bool FT6206Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool FT6206Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool FT6206Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool FT6206Driver::readBlock(uint8_t reg, uint8_t* buffer, size_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)length) != length) {
        return false;
    }
    
    for (size_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
}

#if POCKETOS_FT6206_ENABLE_REGISTER_ACCESS
static const RegisterDesc FT6206_REGISTERS[] = {
    RegisterDesc(0x00, "DEV_MODE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "TD_STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x03, "P1_XH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x04, "P1_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x05, "P1_YH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x06, "P1_YL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xA3, "CHIPID", 1, RegisterAccess::RO, 0x06),
    RegisterDesc(0xA6, "FIRMID", 1, RegisterAccess::RO, 0x00),
};

#define FT6206_REGISTER_COUNT (sizeof(FT6206_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* FT6206Driver::registers(size_t& count) const {
    count = FT6206_REGISTER_COUNT;
    return FT6206_REGISTERS;
}

bool FT6206Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf);
}

bool FT6206Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* FT6206Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < FT6206_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(FT6206_REGISTERS[i].name)) {
            return &FT6206_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
