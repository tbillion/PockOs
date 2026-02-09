#include "lsm9ds1_driver.h"
#include "../driver_config.h"

#if POCKETOS_LSM9DS1_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// LSM9DS1 Accel+Gyro Register addresses
#define LSM9DS1_AG_WHO_AM_I         0x0F
#define LSM9DS1_AG_CTRL_REG1_G      0x10
#define LSM9DS1_AG_CTRL_REG6_XL     0x20
#define LSM9DS1_AG_OUT_TEMP_L       0x15
#define LSM9DS1_AG_OUT_TEMP_H       0x16
#define LSM9DS1_AG_OUT_X_L_G        0x18
#define LSM9DS1_AG_OUT_X_H_G        0x19
#define LSM9DS1_AG_OUT_Y_L_G        0x1A
#define LSM9DS1_AG_OUT_Y_H_G        0x1B
#define LSM9DS1_AG_OUT_Z_L_G        0x1C
#define LSM9DS1_AG_OUT_Z_H_G        0x1D
#define LSM9DS1_AG_OUT_X_L_XL       0x28
#define LSM9DS1_AG_OUT_X_H_XL       0x29
#define LSM9DS1_AG_OUT_Y_L_XL       0x2A
#define LSM9DS1_AG_OUT_Y_H_XL       0x2B
#define LSM9DS1_AG_OUT_Z_L_XL       0x2C
#define LSM9DS1_AG_OUT_Z_H_XL       0x2D

// LSM9DS1 Magnetometer Register addresses
#define LSM9DS1_M_WHO_AM_I          0x0F
#define LSM9DS1_M_CTRL_REG1_M       0x20
#define LSM9DS1_M_CTRL_REG2_M       0x21
#define LSM9DS1_M_CTRL_REG3_M       0x22
#define LSM9DS1_M_OUT_X_L_M         0x28
#define LSM9DS1_M_OUT_X_H_M         0x29
#define LSM9DS1_M_OUT_Y_L_M         0x2A
#define LSM9DS1_M_OUT_Y_H_M         0x2B
#define LSM9DS1_M_OUT_Z_L_M         0x2C
#define LSM9DS1_M_OUT_Z_H_M         0x2D

// WHO_AM_I values
#define LSM9DS1_AG_WHO_AM_I_VALUE   0x68
#define LSM9DS1_M_WHO_AM_I_VALUE    0x3D

#if POCKETOS_LSM9DS1_ENABLE_REGISTER_ACCESS
static const RegisterDesc LSM9DS1_REGISTERS[] = {
    // Accel+Gyro registers
    RegisterDesc(0x0F, "AG_WHO_AM_I", 1, RegisterAccess::RO, 0x68),
    RegisterDesc(0x10, "AG_CTRL_REG1_G", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "AG_CTRL_REG2_G", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "AG_CTRL_REG3_G", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x15, "AG_OUT_TEMP_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x16, "AG_OUT_TEMP_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x18, "AG_OUT_X_L_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x19, "AG_OUT_X_H_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1A, "AG_OUT_Y_L_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1B, "AG_OUT_Y_H_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1C, "AG_OUT_Z_L_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1D, "AG_OUT_Z_H_G", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x20, "AG_CTRL_REG6_XL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "AG_OUT_X_L_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "AG_OUT_X_H_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "AG_OUT_Y_L_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "AG_OUT_Y_H_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "AG_OUT_Z_L_XL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2D, "AG_OUT_Z_H_XL", 1, RegisterAccess::RO, 0x00),
    // Magnetometer registers
    RegisterDesc(0x0F, "M_WHO_AM_I", 1, RegisterAccess::RO, 0x3D),
    RegisterDesc(0x20, "M_CTRL_REG1_M", 1, RegisterAccess::RW, 0x10),
    RegisterDesc(0x21, "M_CTRL_REG2_M", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "M_CTRL_REG3_M", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x28, "M_OUT_X_L_M", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "M_OUT_X_H_M", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "M_OUT_Y_L_M", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "M_OUT_Y_H_M", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "M_OUT_Z_L_M", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2D, "M_OUT_Z_H_M", 1, RegisterAccess::RO, 0x00),
};
#define LSM9DS1_REGISTER_COUNT (sizeof(LSM9DS1_REGISTERS) / sizeof(RegisterDesc))
#endif

LSM9DS1Driver::LSM9DS1Driver() : agAddress(0), magAddress(0), initialized(false),
                                 accelScale(0.061f), gyroScale(8.75f), magScale(0.14f) {}

bool LSM9DS1Driver::init(uint8_t i2cAddress) {
    // The LSM9DS1 has two I2C addresses: AG chip and mag chip
    // i2cAddress is the AG address, mag is always at 0x1E
    agAddress = i2cAddress;
    magAddress = 0x1E;
    
#if POCKETOS_LSM9DS1_ENABLE_LOGGING
    Logger::info("LSM9DS1: Initializing dual-chip 9-DoF IMU");
    Logger::info("LSM9DS1: AG address: 0x" + String(agAddress, HEX));
    Logger::info("LSM9DS1: Mag address: 0x" + String(magAddress, HEX));
#endif
    
    // Check AG WHO_AM_I
    uint8_t agWhoAmI = 0;
    if (!readRegister(agAddress, LSM9DS1_AG_WHO_AM_I, &agWhoAmI)) {
#if POCKETOS_LSM9DS1_ENABLE_LOGGING
        Logger::error("LSM9DS1: Failed to read AG WHO_AM_I at 0x" + String(agAddress, HEX));
#endif
        return false;
    }
    
    if (agWhoAmI != LSM9DS1_AG_WHO_AM_I_VALUE) {
#if POCKETOS_LSM9DS1_ENABLE_LOGGING
        Logger::error("LSM9DS1: Invalid AG WHO_AM_I: 0x" + String(agWhoAmI, HEX));
#endif
        return false;
    }
    
    // Check Mag WHO_AM_I
    uint8_t magWhoAmI = 0;
    if (!readRegister(magAddress, LSM9DS1_M_WHO_AM_I, &magWhoAmI)) {
#if POCKETOS_LSM9DS1_ENABLE_LOGGING
        Logger::error("LSM9DS1: Failed to read Mag WHO_AM_I at 0x" + String(magAddress, HEX));
#endif
        return false;
    }
    
    if (magWhoAmI != LSM9DS1_M_WHO_AM_I_VALUE) {
#if POCKETOS_LSM9DS1_ENABLE_LOGGING
        Logger::error("LSM9DS1: Invalid Mag WHO_AM_I: 0x" + String(magWhoAmI, HEX));
#endif
        return false;
    }
    
#if POCKETOS_LSM9DS1_ENABLE_CONFIGURATION
    // Configure gyroscope: 119Hz, ±245dps
    writeRegister(agAddress, LSM9DS1_AG_CTRL_REG1_G, 0x60);
    gyroScale = 8.75f;  // mdps/LSB for ±245dps
    
    // Configure accelerometer: 119Hz, ±2g
    writeRegister(agAddress, LSM9DS1_AG_CTRL_REG6_XL, 0x60);
    accelScale = 0.061f;  // mg/LSB for ±2g
    
    // Configure magnetometer: continuous mode, high performance, ±4 gauss
    writeRegister(magAddress, LSM9DS1_M_CTRL_REG1_M, 0x7C);  // Temp comp, high perf XY, 80Hz
    writeRegister(magAddress, LSM9DS1_M_CTRL_REG2_M, 0x00);  // ±4 gauss
    writeRegister(magAddress, LSM9DS1_M_CTRL_REG3_M, 0x00);  // Continuous conversion
    magScale = 0.14f;  // mgauss/LSB for ±4 gauss
#else
    // Minimal configuration
    writeRegister(agAddress, LSM9DS1_AG_CTRL_REG1_G, 0x60);
    writeRegister(agAddress, LSM9DS1_AG_CTRL_REG6_XL, 0x60);
    writeRegister(magAddress, LSM9DS1_M_CTRL_REG1_M, 0x7C);
    writeRegister(magAddress, LSM9DS1_M_CTRL_REG2_M, 0x00);
    writeRegister(magAddress, LSM9DS1_M_CTRL_REG3_M, 0x00);
    accelScale = 0.061f;
    gyroScale = 8.75f;
    magScale = 0.14f;
#endif
    
    delay(10);  // Wait for first measurement
    
    initialized = true;
#if POCKETOS_LSM9DS1_ENABLE_LOGGING
    Logger::info("LSM9DS1: Initialized successfully");
#endif
    return true;
}

void LSM9DS1Driver::deinit() {
    if (initialized) {
        writeRegister(agAddress, LSM9DS1_AG_CTRL_REG1_G, 0x00);    // Power down gyro
        writeRegister(agAddress, LSM9DS1_AG_CTRL_REG6_XL, 0x00);   // Power down accel
        writeRegister(magAddress, LSM9DS1_M_CTRL_REG3_M, 0x03);    // Power down mag
    }
    initialized = false;
}

LSM9DS1Data LSM9DS1Driver::readData() {
    LSM9DS1Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read accelerometer data (6 bytes)
    if (readRegisters(agAddress, LSM9DS1_AG_OUT_X_L_XL, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        
        // Convert to m/s² (mg/LSB * 0.001 * 9.81)
        data.accel_x = x * accelScale * 0.001f * 9.81f;
        data.accel_y = y * accelScale * 0.001f * 9.81f;
        data.accel_z = z * accelScale * 0.001f * 9.81f;
    }
    
    // Read gyroscope data (6 bytes)
    if (readRegisters(agAddress, LSM9DS1_AG_OUT_X_L_G, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        
        // Convert to rad/s (mdps/LSB * 0.001 * PI/180)
        float dpsToRad = 0.001f * 3.14159265f / 180.0f;
        data.gyro_x = x * gyroScale * dpsToRad;
        data.gyro_y = y * gyroScale * dpsToRad;
        data.gyro_z = z * gyroScale * dpsToRad;
    }
    
    // Read magnetometer data (6 bytes)
    if (readRegisters(magAddress, LSM9DS1_M_OUT_X_L_M, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        
        // Convert to µT (mgauss/LSB * 0.1)
        data.mag_x = x * magScale * 0.1f;
        data.mag_y = y * magScale * 0.1f;
        data.mag_z = z * magScale * 0.1f;
    }
    
    // Read temperature (2 bytes)
    uint8_t tempBuf[2];
    if (readRegisters(agAddress, LSM9DS1_AG_OUT_TEMP_L, tempBuf, 2)) {
        int16_t temp = (int16_t)((tempBuf[1] << 8) | tempBuf[0]);
        data.temperature = 25.0f + (temp / 16.0f);  // LSB/°C = 16
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema LSM9DS1Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings
    schema.addSetting("driver", ParamType::STRING, true, "lsm9ds1", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_LSM9DS1_TIER_NAME, "", "", "");
    
    // Output signals
    schema.addSignal("accel_x", ParamType::FLOAT, true, "m/s²");
    schema.addSignal("accel_y", ParamType::FLOAT, true, "m/s²");
    schema.addSignal("accel_z", ParamType::FLOAT, true, "m/s²");
    schema.addSignal("gyro_x", ParamType::FLOAT, true, "rad/s");
    schema.addSignal("gyro_y", ParamType::FLOAT, true, "rad/s");
    schema.addSignal("gyro_z", ParamType::FLOAT, true, "rad/s");
    schema.addSignal("mag_x", ParamType::FLOAT, true, "µT");
    schema.addSignal("mag_y", ParamType::FLOAT, true, "µT");
    schema.addSignal("mag_z", ParamType::FLOAT, true, "µT");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    
    return schema;
}

String LSM9DS1Driver::getParameter(const String& name) {
#if POCKETOS_LSM9DS1_ENABLE_CONFIGURATION
    if (name == "accel_range") {
        uint8_t ctrl;
        if (readRegister(agAddress, LSM9DS1_AG_CTRL_REG6_XL, &ctrl)) {
            return String((ctrl >> 3) & 0x03);
        }
    } else if (name == "gyro_range") {
        uint8_t ctrl;
        if (readRegister(agAddress, LSM9DS1_AG_CTRL_REG1_G, &ctrl)) {
            return String((ctrl >> 3) & 0x03);
        }
    } else if (name == "mag_range") {
        uint8_t ctrl;
        if (readRegister(magAddress, LSM9DS1_M_CTRL_REG2_M, &ctrl)) {
            return String((ctrl >> 5) & 0x03);
        }
    }
#endif
    return "";
}

bool LSM9DS1Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_LSM9DS1_ENABLE_CONFIGURATION
    if (name == "accel_range") {
        return setAccelRange(value.toInt());
    } else if (name == "gyro_range") {
        return setGyroRange(value.toInt());
    } else if (name == "mag_range") {
        return setMagRange(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_LSM9DS1_ENABLE_CONFIGURATION
bool LSM9DS1Driver::setAccelRange(uint8_t range) {
    if (!initialized || range > 3) return false;
    
    uint8_t ctrl;
    if (!readRegister(agAddress, LSM9DS1_AG_CTRL_REG6_XL, &ctrl)) return false;
    
    // Clear and set range bits (FS_XL[1:0])
    ctrl = (ctrl & 0xE7) | ((range & 0x03) << 3);
    writeRegister(agAddress, LSM9DS1_AG_CTRL_REG6_XL, ctrl);
    
    // Update scale: 0=±2g, 1=±16g, 2=±4g, 3=±8g
    float scales[] = { 0.061f, 0.732f, 0.122f, 0.244f };
    accelScale = scales[range];
    
    return true;
}

bool LSM9DS1Driver::setGyroRange(uint8_t range) {
    if (!initialized || range > 3) return false;
    
    uint8_t ctrl;
    if (!readRegister(agAddress, LSM9DS1_AG_CTRL_REG1_G, &ctrl)) return false;
    
    // Clear and set range bits (FS_G[1:0])
    ctrl = (ctrl & 0xE7) | ((range & 0x03) << 3);
    writeRegister(agAddress, LSM9DS1_AG_CTRL_REG1_G, ctrl);
    
    // Update scale: 0=±245dps, 1=±500dps, 2=±1000dps, 3=±2000dps
    float scales[] = { 8.75f, 17.50f, 35.0f, 70.0f };
    gyroScale = scales[range];
    
    return true;
}

bool LSM9DS1Driver::setMagRange(uint8_t range) {
    if (!initialized || range > 3) return false;
    
    uint8_t ctrl;
    if (!readRegister(magAddress, LSM9DS1_M_CTRL_REG2_M, &ctrl)) return false;
    
    // Clear and set range bits (FS[1:0])
    ctrl = (ctrl & 0x9F) | ((range & 0x03) << 5);
    writeRegister(magAddress, LSM9DS1_M_CTRL_REG2_M, ctrl);
    
    // Update scale: 0=±4 gauss, 1=±8 gauss, 2=±12 gauss, 3=±16 gauss
    float scales[] = { 0.14f, 0.29f, 0.43f, 0.58f };
    magScale = scales[range];
    
    return true;
}
#endif

#if POCKETOS_LSM9DS1_ENABLE_REGISTER_ACCESS
const RegisterDesc* LSM9DS1Driver::registers(size_t& count) const {
    count = LSM9DS1_REGISTER_COUNT;
    return LSM9DS1_REGISTERS;
}

bool LSM9DS1Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LSM9DS1_REGISTERS, LSM9DS1_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    // Determine which chip based on register address
    uint8_t chipAddr = (reg >= 0x20 && reg <= 0x2D) ? magAddress : agAddress;
    return readRegister(chipAddr, (uint8_t)reg, buf);
}

bool LSM9DS1Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LSM9DS1_REGISTERS, LSM9DS1_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    // Determine which chip based on register address
    uint8_t chipAddr = (reg >= 0x20 && reg <= 0x2D) ? magAddress : agAddress;
    return writeRegister(chipAddr, (uint8_t)reg, buf[0]);
}

const RegisterDesc* LSM9DS1Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(LSM9DS1_REGISTERS, LSM9DS1_REGISTER_COUNT, name);
}
#endif

bool LSM9DS1Driver::writeRegister(uint8_t addr, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool LSM9DS1Driver::readRegister(uint8_t addr, uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    
    if (Wire.requestFrom(addr, (uint8_t)1) != 1) {
        return false;
    }
    
    *value = Wire.read();
    return true;
}

bool LSM9DS1Driver::readRegisters(uint8_t addr, uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    
    if (Wire.requestFrom(addr, (uint8_t)len) != len) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
}

} // namespace PocketOS
