#include "mag3110_driver.h"
#include "../driver_config.h"

#if POCKETOS_MAG3110_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define MAG3110_REG_DR_STATUS     0x00
#define MAG3110_REG_OUT_X_MSB     0x01
#define MAG3110_REG_WHO_AM_I      0x07
#define MAG3110_REG_CTRL_REG1     0x10
#define MAG3110_REG_CTRL_REG2     0x11

#define MAG3110_WHO_AM_I          0xC4

MAG3110Driver::MAG3110Driver() : address(0), initialized(false) {}

bool MAG3110Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MAG3110_ENABLE_LOGGING
    Logger::info("MAG3110: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t whoAmI = 0;
    if (!readRegister(MAG3110_REG_WHO_AM_I, &whoAmI)) {
#if POCKETOS_MAG3110_ENABLE_LOGGING
        Logger::error("MAG3110: Failed to read WHO_AM_I");
#endif
        return false;
    }
    
    if (whoAmI != MAG3110_WHO_AM_I) {
#if POCKETOS_MAG3110_ENABLE_LOGGING
        Logger::error("MAG3110: Invalid WHO_AM_I: 0x" + String(whoAmI, HEX));
#endif
        return false;
    }
    
#if POCKETOS_MAG3110_ENABLE_CONFIGURATION
    writeRegister(MAG3110_REG_CTRL_REG2, 0x80);
    delay(10);
    
    writeRegister(MAG3110_REG_CTRL_REG1, 0x01);
#else
    writeRegister(MAG3110_REG_CTRL_REG1, 0x01);
#endif
    
    initialized = true;
#if POCKETOS_MAG3110_ENABLE_LOGGING
    Logger::info("MAG3110: Initialized successfully");
#endif
    return true;
}

void MAG3110Driver::deinit() {
    if (initialized) {
        writeRegister(MAG3110_REG_CTRL_REG1, 0x00);
    }
    initialized = false;
}

MAG3110Data MAG3110Driver::readData() {
    MAG3110Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t status = 0;
    if (!readRegister(MAG3110_REG_DR_STATUS, &status)) {
        return data;
    }
    
    if (!(status & 0x08)) {
        return data;
    }
    
    uint8_t buffer[6];
    if (!readBlock(MAG3110_REG_OUT_X_MSB, buffer, 6)) {
        return data;
    }
    
    data.x = (buffer[0] << 8) | buffer[1];
    data.y = (buffer[2] << 8) | buffer[3];
    data.z = (buffer[4] << 8) | buffer[5];
    data.valid = true;
    
    return data;
}

CapabilitySchema MAG3110Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "mag3110";
    schema.tier = POCKETOS_MAG3110_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("x", "Magnetic field X", "uT", "-30000-30000"));
    schema.outputs.push_back(OutputDesc("y", "Magnetic field Y", "uT", "-30000-30000"));
    schema.outputs.push_back(OutputDesc("z", "Magnetic field Z", "uT", "-30000-30000"));
    
    return schema;
}

String MAG3110Driver::getParameter(const String& name) {
    return "";
}

bool MAG3110Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool MAG3110Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool MAG3110Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MAG3110Driver::readBlock(uint8_t reg, uint8_t* buffer, size_t length) {
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

#if POCKETOS_MAG3110_ENABLE_REGISTER_ACCESS
static const RegisterDesc MAG3110_REGISTERS[] = {
    RegisterDesc(0x00, "DR_STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "OUT_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "OUT_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x03, "OUT_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x04, "OUT_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x05, "OUT_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x06, "OUT_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x07, "WHO_AM_I", 1, RegisterAccess::RO, 0xC4),
    RegisterDesc(0x10, "CTRL_REG1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "CTRL_REG2", 1, RegisterAccess::RW, 0x00),
};

#define MAG3110_REGISTER_COUNT (sizeof(MAG3110_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* MAG3110Driver::registers(size_t& count) const {
    count = MAG3110_REGISTER_COUNT;
    return MAG3110_REGISTERS;
}

bool MAG3110Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf);
}

bool MAG3110Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MAG3110Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < MAG3110_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(MAG3110_REGISTERS[i].name)) {
            return &MAG3110_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
