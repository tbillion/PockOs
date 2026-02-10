#include "lis3mdl_driver.h"
#include "../driver_config.h"

#if POCKETOS_LIS3MDL_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// LIS3MDL Register addresses
#define LIS3MDL_REG_WHO_AM_I      0x0F
#define LIS3MDL_REG_CTRL_REG1     0x20
#define LIS3MDL_REG_CTRL_REG2     0x21
#define LIS3MDL_REG_CTRL_REG3     0x22
#define LIS3MDL_REG_OUT_X_L       0x28
#define LIS3MDL_REG_OUT_X_H       0x29
#define LIS3MDL_REG_OUT_Y_L       0x2A
#define LIS3MDL_REG_OUT_Y_H       0x2B
#define LIS3MDL_REG_OUT_Z_L       0x2C
#define LIS3MDL_REG_OUT_Z_H       0x2D
#define LIS3MDL_REG_TEMP_OUT_L    0x2E
#define LIS3MDL_REG_TEMP_OUT_H    0x2F

// WHO_AM_I value
#define LIS3MDL_WHO_AM_I_VALUE    0x3D

#if POCKETOS_LIS3MDL_ENABLE_REGISTER_ACCESS
static const RegisterDesc LIS3MDL_REGISTERS[] = {
    RegisterDesc(0x0F, "WHO_AM_I", 1, RegisterAccess::RO, 0x3D),
    RegisterDesc(0x20, "CTRL_REG1", 1, RegisterAccess::RW, 0x10),
    RegisterDesc(0x21, "CTRL_REG2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "CTRL_REG3", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x23, "CTRL_REG4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x24, "CTRL_REG5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "OUT_X_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "OUT_X_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "OUT_Y_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "OUT_Y_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "OUT_Z_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2D, "OUT_Z_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2E, "TEMP_OUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2F, "TEMP_OUT_H", 1, RegisterAccess::RO, 0x00),
};
#define LIS3MDL_REGISTER_COUNT (sizeof(LIS3MDL_REGISTERS) / sizeof(RegisterDesc))
#endif

LIS3MDLDriver::LIS3MDLDriver() : address(0), initialized(false), magScale(0.14607f) {}

bool LIS3MDLDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_LIS3MDL_ENABLE_LOGGING
    Logger::info("LIS3MDL: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check WHO_AM_I register
    uint8_t whoami;
    if (!readRegister(LIS3MDL_REG_WHO_AM_I, &whoami)) {
#if POCKETOS_LIS3MDL_ENABLE_LOGGING
        Logger::error("LIS3MDL: Failed to read WHO_AM_I register");
#endif
        return false;
    }
    
    if (whoami != LIS3MDL_WHO_AM_I_VALUE) {
#if POCKETOS_LIS3MDL_ENABLE_LOGGING
        Logger::error("LIS3MDL: Invalid WHO_AM_I value: 0x" + String(whoami, HEX));
#endif
        return false;
    }
    
#if POCKETOS_LIS3MDL_ENABLE_CONFIGURATION
    // Configure: Ultra-high performance mode for X and Y, 10Hz data rate
    writeRegister(LIS3MDL_REG_CTRL_REG1, 0x70);
    
    // Set scale to ±4 gauss (default)
    writeRegister(LIS3MDL_REG_CTRL_REG2, 0x00);
    magScale = 0.14607f;  // 6842 LSB/gauss -> 0.14607 µT/LSB
    
    // Continuous conversion mode
    writeRegister(LIS3MDL_REG_CTRL_REG3, 0x00);
#else
    // Minimal: continuous mode, default settings
    writeRegister(LIS3MDL_REG_CTRL_REG1, 0x70);
    writeRegister(LIS3MDL_REG_CTRL_REG2, 0x00);
    writeRegister(LIS3MDL_REG_CTRL_REG3, 0x00);
    magScale = 0.14607f;  // ±4 gauss scale
#endif
    
    delay(10);  // Wait for first measurement
    
    initialized = true;
#if POCKETOS_LIS3MDL_ENABLE_LOGGING
    Logger::info("LIS3MDL: Initialized successfully");
#endif
    return true;
}

void LIS3MDLDriver::deinit() {
    if (initialized) {
        writeRegister(LIS3MDL_REG_CTRL_REG3, 0x03);  // Power down mode
    }
    initialized = false;
}

LIS3MDLData LIS3MDLDriver::readData() {
    LIS3MDLData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read 6 bytes starting from OUT_X_L with auto-increment
    if (readRegisters(LIS3MDL_REG_OUT_X_L | 0x80, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        
        data.mag_x = x * magScale;
        data.mag_y = y * magScale;
        data.mag_z = z * magScale;
        
        // Read temperature (2 bytes)
        uint8_t tempBuffer[2];
        if (readRegisters(LIS3MDL_REG_TEMP_OUT_L | 0x80, tempBuffer, 2)) {
            int16_t temp = (int16_t)((tempBuffer[1] << 8) | tempBuffer[0]);
            data.temperature = 25.0f + (temp / 8.0f);  // 8 LSB/°C
        }
        
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema LIS3MDLDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_LIS3MDL_TIER_NAME;
    
    schema.addOutput("mag_x", "number", "µT", "X-axis magnetic field");
    schema.addOutput("mag_y", "number", "µT", "Y-axis magnetic field");
    schema.addOutput("mag_z", "number", "µT", "Z-axis magnetic field");
    schema.addOutput("temperature", "number", "°C", "Temperature");
    
    return schema;
}

String LIS3MDLDriver::getParameter(const String& name) {
    return "";
}

bool LIS3MDLDriver::setParameter(const String& name, const String& value) {
#if POCKETOS_LIS3MDL_ENABLE_CONFIGURATION
    if (name == "data_rate") {
        return setDataRate(value.toInt());
    } else if (name == "scale") {
        return setScale(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_LIS3MDL_ENABLE_CONFIGURATION
bool LIS3MDLDriver::setDataRate(uint8_t rate) {
    if (!initialized || rate > 7) return false;
    
    uint8_t ctrl1;
    if (!readRegister(LIS3MDL_REG_CTRL_REG1, &ctrl1)) return false;
    
    ctrl1 = (ctrl1 & 0xE3) | (rate << 2);
    return writeRegister(LIS3MDL_REG_CTRL_REG1, ctrl1);
}

bool LIS3MDLDriver::setScale(uint8_t scale) {
    if (!initialized || scale > 3) return false;
    
    // Update scale factor: 0=±4G, 1=±8G, 2=±12G, 3=±16G
    const float scales[] = {0.14607f, 0.29214f, 0.43821f, 0.58428f};
    magScale = scales[scale];
    
    writeRegister(LIS3MDL_REG_CTRL_REG2, scale << 5);
    
    return true;
}
#endif

#if POCKETOS_LIS3MDL_ENABLE_REGISTER_ACCESS
const RegisterDesc* LIS3MDLDriver::registers(size_t& count) const {
    count = LIS3MDL_REGISTER_COUNT;
    return LIS3MDL_REGISTERS;
}

bool LIS3MDLDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LIS3MDL_REGISTERS, LIS3MDL_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool LIS3MDLDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LIS3MDL_REGISTERS, LIS3MDL_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* LIS3MDLDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(LIS3MDL_REGISTERS, LIS3MDL_REGISTER_COUNT, name);
}
#endif

bool LIS3MDLDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool LIS3MDLDriver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
        return false;
    }
    
    *value = Wire.read();
    return true;
}

bool LIS3MDLDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)len) != len) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
}

} // namespace PocketOS
