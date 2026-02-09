#include "lsm6dsox_driver.h"
#include "../driver_config.h"

#if POCKETOS_LSM6DSOX_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// LSM6DSOX Register addresses
#define LSM6DSOX_REG_WHO_AM_I       0x0F
#define LSM6DSOX_REG_CTRL1_XL       0x10
#define LSM6DSOX_REG_CTRL2_G        0x11
#define LSM6DSOX_REG_OUT_TEMP_L     0x20
#define LSM6DSOX_REG_OUT_TEMP_H     0x21
#define LSM6DSOX_REG_OUTX_L_G       0x22
#define LSM6DSOX_REG_OUTX_H_G       0x23
#define LSM6DSOX_REG_OUTY_L_G       0x24
#define LSM6DSOX_REG_OUTY_H_G       0x25
#define LSM6DSOX_REG_OUTZ_L_G       0x26
#define LSM6DSOX_REG_OUTZ_H_G       0x27
#define LSM6DSOX_REG_OUTX_L_XL      0x28
#define LSM6DSOX_REG_OUTX_H_XL      0x29
#define LSM6DSOX_REG_OUTY_L_XL      0x2A
#define LSM6DSOX_REG_OUTY_H_XL      0x2B
#define LSM6DSOX_REG_OUTZ_L_XL      0x2C
#define LSM6DSOX_REG_OUTZ_H_XL      0x2D

// WHO_AM_I value
#define LSM6DSOX_WHO_AM_I_VALUE     0x6C

#if POCKETOS_LSM6DSOX_ENABLE_REGISTER_ACCESS
static const RegisterDesc LSM6DSOX_REGISTERS[] = {
    RegisterDesc(0x0F, "WHO_AM_I", 1, RegisterAccess::RO, 0x6C),
    RegisterDesc(0x10, "CTRL1_XL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "CTRL2_G", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "CTRL3_C", 1, RegisterAccess::RW, 0x04),
    RegisterDesc(0x13, "CTRL4_C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x14, "CTRL5_C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x15, "CTRL6_C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x16, "CTRL7_G", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x17, "CTRL8_XL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x18, "CTRL9_XL", 1, RegisterAccess::RW, 0xE0),
    RegisterDesc(0x19, "CTRL10_C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x20, "OUT_TEMP_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x21, "OUT_TEMP_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x22, "OUTX_L_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x23, "OUTX_H_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x24, "OUTY_L_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x25, "OUTY_H_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x26, "OUTZ_L_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x27, "OUTZ_H_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x28, "OUTX_L_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "OUTX_H_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "OUTY_L_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "OUTY_H_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "OUTZ_L_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2D, "OUTZ_H_XL", 1, RegisterAccess::RO, 0x00),
};
#define LSM6DSOX_REGISTER_COUNT (sizeof(LSM6DSOX_REGISTERS) / sizeof(RegisterDesc))
#endif

LSM6DSOXDriver::LSM6DSOXDriver() : address(0), initialized(false), 
                                   accelScale(0.061f), gyroScale(8.75f) {}

bool LSM6DSOXDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_LSM6DSOX_ENABLE_LOGGING
    Logger::info("LSM6DSOX: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check WHO_AM_I
    uint8_t whoAmI = 0;
    if (!readRegister(LSM6DSOX_REG_WHO_AM_I, &whoAmI)) {
#if POCKETOS_LSM6DSOX_ENABLE_LOGGING
        Logger::error("LSM6DSOX: Failed to read WHO_AM_I");
#endif
        return false;
    }
    
    if (whoAmI != LSM6DSOX_WHO_AM_I_VALUE) {
#if POCKETOS_LSM6DSOX_ENABLE_LOGGING
        Logger::error("LSM6DSOX: Invalid WHO_AM_I: 0x" + String(whoAmI, HEX));
#endif
        return false;
    }
    
#if POCKETOS_LSM6DSOX_ENABLE_CONFIGURATION
    // Configure accelerometer: 104Hz, ±2g
    writeRegister(LSM6DSOX_REG_CTRL1_XL, 0x40);
    accelScale = 0.061f;  // mg/LSB for ±2g
    
    // Configure gyroscope: 104Hz, ±250dps
    writeRegister(LSM6DSOX_REG_CTRL2_G, 0x40);
    gyroScale = 8.75f;  // mdps/LSB for ±250dps
#else
    // Minimal configuration: 104Hz for both
    writeRegister(LSM6DSOX_REG_CTRL1_XL, 0x40);
    writeRegister(LSM6DSOX_REG_CTRL2_G, 0x40);
    accelScale = 0.061f;
    gyroScale = 8.75f;
#endif
    
    delay(10);  // Wait for first measurement
    
    initialized = true;
#if POCKETOS_LSM6DSOX_ENABLE_LOGGING
    Logger::info("LSM6DSOX: Initialized successfully");
#endif
    return true;
}

void LSM6DSOXDriver::deinit() {
    if (initialized) {
        writeRegister(LSM6DSOX_REG_CTRL1_XL, 0x00);  // Power down accel
        writeRegister(LSM6DSOX_REG_CTRL2_G, 0x00);   // Power down gyro
    }
    initialized = false;
}

LSM6DSOXData LSM6DSOXDriver::readData() {
    LSM6DSOXData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read accelerometer data (6 bytes)
    if (readRegisters(LSM6DSOX_REG_OUTX_L_XL, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        
        // Convert to m/s² (mg/LSB * 0.001 * 9.81)
        data.accel_x = x * accelScale * 0.001f * 9.81f;
        data.accel_y = y * accelScale * 0.001f * 9.81f;
        data.accel_z = z * accelScale * 0.001f * 9.81f;
    }
    
    // Read gyroscope data (6 bytes)
    if (readRegisters(LSM6DSOX_REG_OUTX_L_G, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        
        // Convert to rad/s (mdps/LSB * 0.001 * PI/180)
        float dpsToRad = 0.001f * 3.14159265f / 180.0f;
        data.gyro_x = x * gyroScale * dpsToRad;
        data.gyro_y = y * gyroScale * dpsToRad;
        data.gyro_z = z * gyroScale * dpsToRad;
    }
    
    // Read temperature (2 bytes)
    uint8_t tempBuf[2];
    if (readRegisters(LSM6DSOX_REG_OUT_TEMP_L, tempBuf, 2)) {
        int16_t temp = (int16_t)((tempBuf[1] << 8) | tempBuf[0]);
        data.temperature = 25.0f + (temp / 256.0f);  // LSB/°C = 256
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema LSM6DSOXDriver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings
    schema.addSetting("driver", ParamType::STRING, true, "lsm6dsox", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_LSM6DSOX_TIER_NAME, "", "", "");
    
    // Output signals
    schema.addSignal("accel_x", ParamType::FLOAT, true, "m/s²");
    schema.addSignal("accel_y", ParamType::FLOAT, true, "m/s²");
    schema.addSignal("accel_z", ParamType::FLOAT, true, "m/s²");
    schema.addSignal("gyro_x", ParamType::FLOAT, true, "rad/s");
    schema.addSignal("gyro_y", ParamType::FLOAT, true, "rad/s");
    schema.addSignal("gyro_z", ParamType::FLOAT, true, "rad/s");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    
    return schema;
}

String LSM6DSOXDriver::getParameter(const String& name) {
#if POCKETOS_LSM6DSOX_ENABLE_CONFIGURATION
    if (name == "accel_range") {
        uint8_t ctrl;
        if (readRegister(LSM6DSOX_REG_CTRL1_XL, &ctrl)) {
            return String((ctrl >> 2) & 0x03);
        }
    } else if (name == "gyro_range") {
        uint8_t ctrl;
        if (readRegister(LSM6DSOX_REG_CTRL2_G, &ctrl)) {
            return String((ctrl >> 2) & 0x03);
        }
    }
#endif
    return "";
}

bool LSM6DSOXDriver::setParameter(const String& name, const String& value) {
#if POCKETOS_LSM6DSOX_ENABLE_CONFIGURATION
    if (name == "accel_range") {
        return setAccelRange(value.toInt());
    } else if (name == "gyro_range") {
        return setGyroRange(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_LSM6DSOX_ENABLE_CONFIGURATION
bool LSM6DSOXDriver::setAccelRange(uint8_t range) {
    if (!initialized || range > 3) return false;
    
    uint8_t ctrl;
    if (!readRegister(LSM6DSOX_REG_CTRL1_XL, &ctrl)) return false;
    
    // Clear and set range bits (FS_XL[1:0])
    ctrl = (ctrl & 0xF3) | ((range & 0x03) << 2);
    writeRegister(LSM6DSOX_REG_CTRL1_XL, ctrl);
    
    // Update scale: 0=±2g, 1=±16g, 2=±4g, 3=±8g
    float scales[] = { 0.061f, 0.488f, 0.122f, 0.244f };
    accelScale = scales[range];
    
    return true;
}

bool LSM6DSOXDriver::setGyroRange(uint8_t range) {
    if (!initialized || range > 3) return false;
    
    uint8_t ctrl;
    if (!readRegister(LSM6DSOX_REG_CTRL2_G, &ctrl)) return false;
    
    // Clear and set range bits (FS_G[1:0])
    ctrl = (ctrl & 0xF3) | ((range & 0x03) << 2);
    writeRegister(LSM6DSOX_REG_CTRL2_G, ctrl);
    
    // Update scale: 0=±250dps, 1=±500dps, 2=±1000dps, 3=±2000dps
    float scales[] = { 8.75f, 17.50f, 35.0f, 70.0f };
    gyroScale = scales[range];
    
    return true;
}
#endif

#if POCKETOS_LSM6DSOX_ENABLE_REGISTER_ACCESS
const RegisterDesc* LSM6DSOXDriver::registers(size_t& count) const {
    count = LSM6DSOX_REGISTER_COUNT;
    return LSM6DSOX_REGISTERS;
}

bool LSM6DSOXDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LSM6DSOX_REGISTERS, LSM6DSOX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool LSM6DSOXDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LSM6DSOX_REGISTERS, LSM6DSOX_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* LSM6DSOXDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(LSM6DSOX_REGISTERS, LSM6DSOX_REGISTER_COUNT, name);
}
#endif

bool LSM6DSOXDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool LSM6DSOXDriver::readRegister(uint8_t reg, uint8_t* value) {
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

bool LSM6DSOXDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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
