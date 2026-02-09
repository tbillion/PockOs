#include "lis2dh12_driver.h"
#include "../driver_config.h"

#if POCKETOS_LIS2DH12_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// LIS2DH12 Register addresses
#define LIS2DH12_REG_WHO_AM_I      0x0F
#define LIS2DH12_REG_CTRL_REG1     0x20
#define LIS2DH12_REG_OUT_X_L       0x28
#define LIS2DH12_REG_OUT_X_H       0x29
#define LIS2DH12_REG_OUT_Y_L       0x2A
#define LIS2DH12_REG_OUT_Y_H       0x2B
#define LIS2DH12_REG_OUT_Z_L       0x2C
#define LIS2DH12_REG_OUT_Z_H       0x2D
#define LIS2DH12_REG_TEMP_OUT_L    0x0C
#define LIS2DH12_REG_TEMP_OUT_H    0x0D

// WHO_AM_I value
#define LIS2DH12_WHO_AM_I_VALUE    0x33

#if POCKETOS_LIS2DH12_ENABLE_REGISTER_ACCESS
static const RegisterDesc LIS2DH12_REGISTERS[] = {
    RegisterDesc(0x0F, "WHO_AM_I", 1, RegisterAccess::RO, 0x33),
    RegisterDesc(0x20, "CTRL_REG1", 1, RegisterAccess::RW, 0x07),
    RegisterDesc(0x21, "CTRL_REG2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "CTRL_REG3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x23, "CTRL_REG4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x24, "CTRL_REG5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x25, "CTRL_REG6", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "OUT_X_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "OUT_X_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "OUT_Y_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "OUT_Y_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "OUT_Z_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2D, "OUT_Z_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "TEMP_OUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "TEMP_OUT_H", 1, RegisterAccess::RO, 0x00),
};
#define LIS2DH12_REGISTER_COUNT (sizeof(LIS2DH12_REGISTERS) / sizeof(RegisterDesc))
#endif

LIS2DH12Driver::LIS2DH12Driver() : address(0), initialized(false), accelScale(0.001f) {}

bool LIS2DH12Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_LIS2DH12_ENABLE_LOGGING
    Logger::info("LIS2DH12: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check WHO_AM_I register
    uint8_t whoami;
    if (!readRegister(LIS2DH12_REG_WHO_AM_I, &whoami)) {
#if POCKETOS_LIS2DH12_ENABLE_LOGGING
        Logger::error("LIS2DH12: Failed to read WHO_AM_I register");
#endif
        return false;
    }
    
    if (whoami != LIS2DH12_WHO_AM_I_VALUE) {
#if POCKETOS_LIS2DH12_ENABLE_LOGGING
        Logger::error("LIS2DH12: Invalid WHO_AM_I value: 0x" + String(whoami, HEX));
#endif
        return false;
    }
    
#if POCKETOS_LIS2DH12_ENABLE_CONFIGURATION
    // Configure: 100Hz data rate, normal mode, all axes enabled
    writeRegister(LIS2DH12_REG_CTRL_REG1, 0x57);
    
    // Set scale to ±2g (default)
    accelScale = 0.001f;  // 1 mg/LSB = 0.001 g/LSB * 9.81 m/s²/g
#else
    // Minimal: 100Hz, normal mode, all axes enabled
    writeRegister(LIS2DH12_REG_CTRL_REG1, 0x57);
    accelScale = 0.001f;  // ±2g scale
#endif
    
    delay(10);  // Wait for first measurement
    
    initialized = true;
#if POCKETOS_LIS2DH12_ENABLE_LOGGING
    Logger::info("LIS2DH12: Initialized successfully");
#endif
    return true;
}

void LIS2DH12Driver::deinit() {
    if (initialized) {
        writeRegister(LIS2DH12_REG_CTRL_REG1, 0x00);  // Power down
    }
    initialized = false;
}

LIS2DH12Data LIS2DH12Driver::readData() {
    LIS2DH12Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read 6 bytes starting from OUT_X_L with auto-increment
    if (readRegisters(LIS2DH12_REG_OUT_X_L | 0x80, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        
        // Convert to m/s² (accelScale is in g/LSB, multiply by 9.81 to get m/s²)
        data.accel_x = (x >> 4) * accelScale * 9.81f;
        data.accel_y = (y >> 4) * accelScale * 9.81f;
        data.accel_z = (z >> 4) * accelScale * 9.81f;
        
        // Read temperature (2 bytes)
        uint8_t tempBuffer[2];
        if (readRegisters(LIS2DH12_REG_TEMP_OUT_L | 0x80, tempBuffer, 2)) {
            int16_t temp = (int16_t)((tempBuffer[1] << 8) | tempBuffer[0]);
            data.temperature = 25.0f + (temp >> 8);  // Temperature offset from 25°C
        }
        
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema LIS2DH12Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_LIS2DH12_TIER_NAME;
    
    schema.addOutput("accel_x", "number", "m/s²", "X-axis acceleration");
    schema.addOutput("accel_y", "number", "m/s²", "Y-axis acceleration");
    schema.addOutput("accel_z", "number", "m/s²", "Z-axis acceleration");
    schema.addOutput("temperature", "number", "°C", "Temperature");
    
    return schema;
}

String LIS2DH12Driver::getParameter(const String& name) {
    return "";
}

bool LIS2DH12Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_LIS2DH12_ENABLE_CONFIGURATION
    if (name == "data_rate") {
        return setDataRate(value.toInt());
    } else if (name == "scale") {
        return setScale(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_LIS2DH12_ENABLE_CONFIGURATION
bool LIS2DH12Driver::setDataRate(uint8_t rate) {
    if (!initialized || rate > 9) return false;
    
    uint8_t ctrl1;
    if (!readRegister(LIS2DH12_REG_CTRL_REG1, &ctrl1)) return false;
    
    ctrl1 = (ctrl1 & 0x0F) | (rate << 4);
    return writeRegister(LIS2DH12_REG_CTRL_REG1, ctrl1);
}

bool LIS2DH12Driver::setScale(uint8_t scale) {
    if (!initialized || scale > 3) return false;
    
    // Update scale factor: 0=±2g, 1=±4g, 2=±8g, 3=±16g
    const float scales[] = {0.001f, 0.002f, 0.004f, 0.012f};
    accelScale = scales[scale];
    
    return true;
}
#endif

#if POCKETOS_LIS2DH12_ENABLE_REGISTER_ACCESS
const RegisterDesc* LIS2DH12Driver::registers(size_t& count) const {
    count = LIS2DH12_REGISTER_COUNT;
    return LIS2DH12_REGISTERS;
}

bool LIS2DH12Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LIS2DH12_REGISTERS, LIS2DH12_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool LIS2DH12Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LIS2DH12_REGISTERS, LIS2DH12_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* LIS2DH12Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(LIS2DH12_REGISTERS, LIS2DH12_REGISTER_COUNT, name);
}
#endif

bool LIS2DH12Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool LIS2DH12Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool LIS2DH12Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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
