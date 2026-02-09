#include "qmc5883l_driver.h"
#include "../driver_config.h"

#if POCKETOS_QMC5883L_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define QMC5883L_REG_X_LSB        0x00
#define QMC5883L_REG_STATUS       0x06
#define QMC5883L_REG_CONTROL1     0x09
#define QMC5883L_REG_CONTROL2     0x0A
#define QMC5883L_REG_PERIOD       0x0B
#define QMC5883L_REG_CHIP_ID      0x0D

#define QMC5883L_CHIP_ID          0xFF

QMC5883LDriver::QMC5883LDriver() : address(0), initialized(false) {}

bool QMC5883LDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_QMC5883L_ENABLE_LOGGING
    Logger::info("QMC5883L: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t chipId = 0;
    if (!readRegister(QMC5883L_REG_CHIP_ID, &chipId)) {
#if POCKETOS_QMC5883L_ENABLE_LOGGING
        Logger::error("QMC5883L: Failed to read chip ID");
#endif
        return false;
    }
    
    if (chipId != QMC5883L_CHIP_ID) {
#if POCKETOS_QMC5883L_ENABLE_LOGGING
        Logger::error("QMC5883L: Invalid chip ID: 0x" + String(chipId, HEX));
#endif
        return false;
    }
    
#if POCKETOS_QMC5883L_ENABLE_CONFIGURATION
    writeRegister(QMC5883L_REG_CONTROL2, 0x80);
    delay(10);
    
    writeRegister(QMC5883L_REG_PERIOD, 0x01);
    writeRegister(QMC5883L_REG_CONTROL1, 0x1D);
#else
    writeRegister(QMC5883L_REG_CONTROL1, 0x01);
#endif
    
    initialized = true;
#if POCKETOS_QMC5883L_ENABLE_LOGGING
    Logger::info("QMC5883L: Initialized successfully");
#endif
    return true;
}

void QMC5883LDriver::deinit() {
    if (initialized) {
        writeRegister(QMC5883L_REG_CONTROL1, 0x00);
    }
    initialized = false;
}

QMC5883LData QMC5883LDriver::readData() {
    QMC5883LData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t status = 0;
    if (!readRegister(QMC5883L_REG_STATUS, &status)) {
        return data;
    }
    
    if (!(status & 0x01)) {
        return data;
    }
    
    uint8_t buffer[6];
    if (!readBlock(QMC5883L_REG_X_LSB, buffer, 6)) {
        return data;
    }
    
    data.x = (buffer[1] << 8) | buffer[0];
    data.y = (buffer[3] << 8) | buffer[2];
    data.z = (buffer[5] << 8) | buffer[4];
    data.valid = true;
    
    return data;
}

CapabilitySchema QMC5883LDriver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "qmc5883l";
    schema.tier = POCKETOS_QMC5883L_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("x", "Magnetic field X", "uT", "-32768-32767"));
    schema.outputs.push_back(OutputDesc("y", "Magnetic field Y", "uT", "-32768-32767"));
    schema.outputs.push_back(OutputDesc("z", "Magnetic field Z", "uT", "-32768-32767"));
    
    return schema;
}

String QMC5883LDriver::getParameter(const String& name) {
    return "";
}

bool QMC5883LDriver::setParameter(const String& name, const String& value) {
    return false;
}

bool QMC5883LDriver::readRegister(uint8_t reg, uint8_t* value) {
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

bool QMC5883LDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool QMC5883LDriver::readBlock(uint8_t reg, uint8_t* buffer, size_t length) {
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

#if POCKETOS_QMC5883L_ENABLE_REGISTER_ACCESS
static const RegisterDesc QMC5883L_REGISTERS[] = {
    RegisterDesc(0x00, "X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x03, "Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x04, "Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x05, "Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x06, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "CONTROL1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "CONTROL2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "PERIOD", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "CHIP_ID", 1, RegisterAccess::RO, 0xFF),
};

#define QMC5883L_REGISTER_COUNT (sizeof(QMC5883L_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* QMC5883LDriver::registers(size_t& count) const {
    count = QMC5883L_REGISTER_COUNT;
    return QMC5883L_REGISTERS;
}

bool QMC5883LDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf);
}

bool QMC5883LDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* QMC5883LDriver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < QMC5883L_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(QMC5883L_REGISTERS[i].name)) {
            return &QMC5883L_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
