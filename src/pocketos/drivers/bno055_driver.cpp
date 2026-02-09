#include "bno055_driver.h"
#include "../driver_config.h"

#if POCKETOS_BNO055_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// BNO055 Register addresses
#define BNO055_REG_CHIP_ID          0x00
#define BNO055_REG_ACC_ID           0x01
#define BNO055_REG_MAG_ID           0x02
#define BNO055_REG_GYR_ID           0x03
#define BNO055_REG_PAGE_ID          0x07
#define BNO055_REG_ACCEL_DATA_X_LSB 0x08
#define BNO055_REG_MAG_DATA_X_LSB   0x0E
#define BNO055_REG_GYRO_DATA_X_LSB  0x14
#define BNO055_REG_EULER_H_LSB      0x1A
#define BNO055_REG_QUAT_DATA_W_LSB  0x20
#define BNO055_REG_TEMP             0x34
#define BNO055_REG_CALIB_STAT       0x35
#define BNO055_REG_SYS_TRIGGER      0x3F
#define BNO055_REG_OPR_MODE         0x3D
#define BNO055_REG_PWR_MODE         0x3E

// Chip ID
#define BNO055_CHIP_ID              0xA0

// Operation modes
#define BNO055_MODE_CONFIG          0x00
#define BNO055_MODE_NDOF            0x0C  // 9-DoF sensor fusion

#if POCKETOS_BNO055_ENABLE_REGISTER_ACCESS
static const RegisterDesc BNO055_REGISTERS[] = {
    RegisterDesc(0x00, "CHIP_ID", 1, RegisterAccess::RO, 0xA0),
    RegisterDesc(0x01, "ACC_ID", 1, RegisterAccess::RO, 0xFB),
    RegisterDesc(0x02, "MAG_ID", 1, RegisterAccess::RO, 0x32),
    RegisterDesc(0x03, "GYR_ID", 1, RegisterAccess::RO, 0x0F),
    RegisterDesc(0x07, "PAGE_ID", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "ACC_DATA_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "ACC_DATA_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "ACC_DATA_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0B, "ACC_DATA_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "ACC_DATA_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "ACC_DATA_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0E, "MAG_DATA_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0F, "MAG_DATA_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x10, "MAG_DATA_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x11, "MAG_DATA_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x12, "MAG_DATA_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x13, "MAG_DATA_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x14, "GYR_DATA_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x15, "GYR_DATA_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x16, "GYR_DATA_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x17, "GYR_DATA_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x18, "GYR_DATA_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x19, "GYR_DATA_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1A, "EUL_HEADING_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1B, "EUL_HEADING_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1C, "EUL_ROLL_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1D, "EUL_ROLL_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1E, "EUL_PITCH_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1F, "EUL_PITCH_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x20, "QUA_DATA_W_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x21, "QUA_DATA_W_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x22, "QUA_DATA_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x23, "QUA_DATA_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x24, "QUA_DATA_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x25, "QUA_DATA_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x26, "QUA_DATA_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x27, "QUA_DATA_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x34, "TEMP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x35, "CALIB_STAT", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x3D, "OPR_MODE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3E, "PWR_MODE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3F, "SYS_TRIGGER", 1, RegisterAccess::WO, 0x00),
};
#define BNO055_REGISTER_COUNT (sizeof(BNO055_REGISTERS) / sizeof(RegisterDesc))
#endif

BNO055Driver::BNO055Driver() : address(0), initialized(false) {}

bool BNO055Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_BNO055_ENABLE_LOGGING
    Logger::info("BNO055: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check chip ID
    uint8_t chipId = 0;
    if (!readRegister(BNO055_REG_CHIP_ID, &chipId)) {
#if POCKETOS_BNO055_ENABLE_LOGGING
        Logger::error("BNO055: Failed to read chip ID");
#endif
        return false;
    }
    
    if (chipId != BNO055_CHIP_ID) {
#if POCKETOS_BNO055_ENABLE_LOGGING
        Logger::error("BNO055: Invalid chip ID: 0x" + String(chipId, HEX));
#endif
        return false;
    }
    
#if POCKETOS_BNO055_ENABLE_CONFIGURATION
    // Set to config mode
    writeRegister(BNO055_REG_OPR_MODE, BNO055_MODE_CONFIG);
    delay(25);
    
    // Reset
    writeRegister(BNO055_REG_SYS_TRIGGER, 0x20);
    delay(650);  // Wait for reset
    
    // Check chip ID again after reset
    if (!readRegister(BNO055_REG_CHIP_ID, &chipId) || chipId != BNO055_CHIP_ID) {
#if POCKETOS_BNO055_ENABLE_LOGGING
        Logger::error("BNO055: Failed after reset");
#endif
        return false;
    }
    
    // Set to normal power mode
    writeRegister(BNO055_REG_PWR_MODE, 0x00);
    delay(10);
    
    // Set operation mode to NDOF (9-DoF sensor fusion)
    writeRegister(BNO055_REG_OPR_MODE, BNO055_MODE_NDOF);
    delay(20);
#else
    // Minimal config: just set NDOF mode
    writeRegister(BNO055_REG_OPR_MODE, BNO055_MODE_NDOF);
    delay(20);
#endif
    
    initialized = true;
#if POCKETOS_BNO055_ENABLE_LOGGING
    Logger::info("BNO055: Initialized successfully");
#endif
    return true;
}

void BNO055Driver::deinit() {
    if (initialized) {
        writeRegister(BNO055_REG_OPR_MODE, BNO055_MODE_CONFIG);
    }
    initialized = false;
}

BNO055Data BNO055Driver::readData() {
    BNO055Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read accelerometer data (6 bytes)
    if (readRegisters(BNO055_REG_ACCEL_DATA_X_LSB, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        data.accel_x = x / 100.0f;  // Convert to m/s²
        data.accel_y = y / 100.0f;
        data.accel_z = z / 100.0f;
    }
    
    // Read magnetometer data (6 bytes)
    if (readRegisters(BNO055_REG_MAG_DATA_X_LSB, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        data.mag_x = x / 16.0f;  // Convert to µT
        data.mag_y = y / 16.0f;
        data.mag_z = z / 16.0f;
    }
    
    // Read gyroscope data (6 bytes)
    if (readRegisters(BNO055_REG_GYRO_DATA_X_LSB, buffer, 6)) {
        int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);
        data.gyro_x = x / 900.0f;  // Convert to rad/s
        data.gyro_y = y / 900.0f;
        data.gyro_z = z / 900.0f;
    }
    
    // Read Euler angles (6 bytes)
    if (readRegisters(BNO055_REG_EULER_H_LSB, buffer, 6)) {
        int16_t h = (int16_t)((buffer[1] << 8) | buffer[0]);
        int16_t r = (int16_t)((buffer[3] << 8) | buffer[2]);
        int16_t p = (int16_t)((buffer[5] << 8) | buffer[4]);
        data.euler_heading = h / 16.0f;  // Convert to degrees
        data.euler_roll = r / 16.0f;
        data.euler_pitch = p / 16.0f;
    }
    
    // Read quaternion (8 bytes)
    uint8_t qbuffer[8];
    if (readRegisters(BNO055_REG_QUAT_DATA_W_LSB, qbuffer, 8)) {
        int16_t w = (int16_t)((qbuffer[1] << 8) | qbuffer[0]);
        int16_t x = (int16_t)((qbuffer[3] << 8) | qbuffer[2]);
        int16_t y = (int16_t)((qbuffer[5] << 8) | qbuffer[4]);
        int16_t z = (int16_t)((qbuffer[7] << 8) | qbuffer[6]);
        data.quat_w = w / 16384.0f;
        data.quat_x = x / 16384.0f;
        data.quat_y = y / 16384.0f;
        data.quat_z = z / 16384.0f;
    }
    
    // Read temperature
    uint8_t temp;
    if (readRegister(BNO055_REG_TEMP, &temp)) {
        data.temperature = (int8_t)temp;  // Already in °C
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema BNO055Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings
    schema.addSetting("driver", ParamType::STRING, false, 0, 0, 0, "");
    schema.addSetting("tier", ParamType::STRING, false, 0, 0, 0, "");
    
    // Output signals
    schema.addSignal("accel_x", ParamType::FLOAT, false, "m/s²");
    schema.addSignal("accel_y", ParamType::FLOAT, false, "m/s²");
    schema.addSignal("accel_z", ParamType::FLOAT, false, "m/s²");
    schema.addSignal("gyro_x", ParamType::FLOAT, false, "rad/s");
    schema.addSignal("gyro_y", ParamType::FLOAT, false, "rad/s");
    schema.addSignal("gyro_z", ParamType::FLOAT, false, "rad/s");
    schema.addSignal("mag_x", ParamType::FLOAT, false, "µT");
    schema.addSignal("mag_y", ParamType::FLOAT, false, "µT");
    schema.addSignal("mag_z", ParamType::FLOAT, false, "µT");
    schema.addSignal("euler_heading", ParamType::FLOAT, false, "°");
    schema.addSignal("euler_roll", ParamType::FLOAT, false, "°");
    schema.addSignal("euler_pitch", ParamType::FLOAT, false, "°");
    schema.addSignal("quat_w", ParamType::FLOAT, false, "");
    schema.addSignal("quat_x", ParamType::FLOAT, false, "");
    schema.addSignal("quat_y", ParamType::FLOAT, false, "");
    schema.addSignal("quat_z", ParamType::FLOAT, false, "");
    schema.addSignal("temperature", ParamType::FLOAT, false, "°C");
    
    return schema;
}

String BNO055Driver::getParameter(const String& name) {
#if POCKETOS_BNO055_ENABLE_CONFIGURATION
    if (name == "operation_mode") {
        return String(getOperationMode());
    } else if (name == "calib_status") {
        return String(getCalibrationStatus());
    }
#endif
    return "";
}

bool BNO055Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_BNO055_ENABLE_CONFIGURATION
    if (name == "operation_mode") {
        return setOperationMode(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_BNO055_ENABLE_CONFIGURATION
bool BNO055Driver::setOperationMode(uint8_t mode) {
    if (!initialized) return false;
    
    // Switch to config mode first
    writeRegister(BNO055_REG_OPR_MODE, BNO055_MODE_CONFIG);
    delay(25);
    
    // Set new mode
    writeRegister(BNO055_REG_OPR_MODE, mode);
    delay(20);
    
    return true;
}

uint8_t BNO055Driver::getOperationMode() {
    if (!initialized) return 0;
    
    uint8_t mode = 0;
    readRegister(BNO055_REG_OPR_MODE, &mode);
    return mode;
}

bool BNO055Driver::calibrate() {
    // BNO055 auto-calibrates in fusion modes
    // This function can be extended to save/load calibration data
    return initialized;
}

uint8_t BNO055Driver::getCalibrationStatus() {
    if (!initialized) return 0;
    
    uint8_t status = 0;
    readRegister(BNO055_REG_CALIB_STAT, &status);
    return status;
}
#endif

#if POCKETOS_BNO055_ENABLE_REGISTER_ACCESS
const RegisterDesc* BNO055Driver::registers(size_t& count) const {
    count = BNO055_REGISTER_COUNT;
    return BNO055_REGISTERS;
}

bool BNO055Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        BNO055_REGISTERS, BNO055_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool BNO055Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        BNO055_REGISTERS, BNO055_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* BNO055Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(BNO055_REGISTERS, BNO055_REGISTER_COUNT, name);
}
#endif

bool BNO055Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool BNO055Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool BNO055Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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
