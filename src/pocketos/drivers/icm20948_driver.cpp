#include "icm20948_driver.h"
#include "../driver_config.h"

#if POCKETOS_ICM20948_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// ICM20948 Register addresses (Bank 0)
#define ICM20948_REG_WHO_AM_I       0x00
#define ICM20948_REG_PWR_MGMT_1     0x06
#define ICM20948_REG_PWR_MGMT_2     0x07
#define ICM20948_REG_ACCEL_XOUT_H   0x2D
#define ICM20948_REG_GYRO_XOUT_H    0x33
#define ICM20948_REG_TEMP_OUT_H     0x39
#define ICM20948_REG_REG_BANK_SEL   0x7F

// Bank 2
#define ICM20948_REG_GYRO_CONFIG_1  0x01
#define ICM20948_REG_ACCEL_CONFIG   0x14

// WHO_AM_I value
#define ICM20948_WHO_AM_I_VALUE     0xEA

#if POCKETOS_ICM20948_ENABLE_REGISTER_ACCESS
static const RegisterDesc ICM20948_REGISTERS[] = {
    RegisterDesc(0x00, "WHO_AM_I", 1, RegisterAccess::RO, 0xEA),
    RegisterDesc(0x06, "PWR_MGMT_1", 1, RegisterAccess::RW, 0x41),
    RegisterDesc(0x07, "PWR_MGMT_2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2D, "ACCEL_XOUT_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2E, "ACCEL_XOUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2F, "ACCEL_YOUT_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x30, "ACCEL_YOUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x31, "ACCEL_ZOUT_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x32, "ACCEL_ZOUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x33, "GYRO_XOUT_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x34, "GYRO_XOUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x35, "GYRO_YOUT_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x36, "GYRO_YOUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x37, "GYRO_ZOUT_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x38, "GYRO_ZOUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x39, "TEMP_OUT_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x3A, "TEMP_OUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x7F, "REG_BANK_SEL", 1, RegisterAccess::RW, 0x00),
};
#define ICM20948_REGISTER_COUNT (sizeof(ICM20948_REGISTERS) / sizeof(RegisterDesc))
#endif

ICM20948Driver::ICM20948Driver() : address(0), initialized(false), 
                                    accelScale(1.0f), gyroScale(1.0f) {}

bool ICM20948Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_ICM20948_ENABLE_LOGGING
    Logger::info("ICM20948: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Select bank 0
    selectBank(0);
    
    // Check WHO_AM_I
    uint8_t whoAmI = 0;
    if (!readRegister(ICM20948_REG_WHO_AM_I, &whoAmI)) {
#if POCKETOS_ICM20948_ENABLE_LOGGING
        Logger::error("ICM20948: Failed to read WHO_AM_I");
#endif
        return false;
    }
    
    if (whoAmI != ICM20948_WHO_AM_I_VALUE) {
#if POCKETOS_ICM20948_ENABLE_LOGGING
        Logger::error("ICM20948: Invalid WHO_AM_I: 0x" + String(whoAmI, HEX));
#endif
        return false;
    }
    
#if POCKETOS_ICM20948_ENABLE_CONFIGURATION
    // Reset device
    writeRegister(ICM20948_REG_PWR_MGMT_1, 0x80);
    delay(100);
    
    // Wake up and use best available clock
    writeRegister(ICM20948_REG_PWR_MGMT_1, 0x01);
    delay(10);
    
    // Enable accel and gyro
    writeRegister(ICM20948_REG_PWR_MGMT_2, 0x00);
    delay(10);
    
    // Configure accel and gyro (Bank 2)
    selectBank(2);
    writeRegister(ICM20948_REG_ACCEL_CONFIG, 0x01);  // ±4g
    writeRegister(ICM20948_REG_GYRO_CONFIG_1, 0x01);  // ±500 dps
    selectBank(0);
    
    accelScale = 4.0f / 32768.0f * 9.81f;  // ±4g to m/s²
    gyroScale = 500.0f / 32768.0f * 0.017453293f;  // ±500dps to rad/s
#else
    // Minimal: wake up
    writeRegister(ICM20948_REG_PWR_MGMT_1, 0x01);
    writeRegister(ICM20948_REG_PWR_MGMT_2, 0x00);
    delay(10);
    
    accelScale = 2.0f / 32768.0f * 9.81f;  // Default ±2g to m/s²
    gyroScale = 250.0f / 32768.0f * 0.017453293f;  // Default ±250dps to rad/s
#endif
    
    initialized = true;
#if POCKETOS_ICM20948_ENABLE_LOGGING
    Logger::info("ICM20948: Initialized successfully");
#endif
    return true;
}

void ICM20948Driver::deinit() {
    if (initialized) {
        writeRegister(ICM20948_REG_PWR_MGMT_1, 0x40);  // Sleep
    }
    initialized = false;
}

ICM20948Data ICM20948Driver::readData() {
    ICM20948Data data;
    
    if (!initialized) {
        return data;
    }
    
    selectBank(0);
    
    uint8_t buffer[14];
    
    // Read accel (6 bytes) + temp (2 bytes) + gyro (6 bytes)
    if (readRegisters(ICM20948_REG_ACCEL_XOUT_H, buffer, 14)) {
        int16_t ax = (int16_t)((buffer[0] << 8) | buffer[1]);
        int16_t ay = (int16_t)((buffer[2] << 8) | buffer[3]);
        int16_t az = (int16_t)((buffer[4] << 8) | buffer[5]);
        
        data.accel_x = ax * accelScale;
        data.accel_y = ay * accelScale;
        data.accel_z = az * accelScale;
        
        int16_t temp = (int16_t)((buffer[6] << 8) | buffer[7]);
        data.temperature = (temp / 333.87f) + 21.0f;
        
        int16_t gx = (int16_t)((buffer[8] << 8) | buffer[9]);
        int16_t gy = (int16_t)((buffer[10] << 8) | buffer[11]);
        int16_t gz = (int16_t)((buffer[12] << 8) | buffer[13]);
        
        data.gyro_x = gx * gyroScale;
        data.gyro_y = gy * gyroScale;
        data.gyro_z = gz * gyroScale;
        
        data.valid = true;
    }
    
    // Read magnetometer
    readMagnetometer(data.mag_x, data.mag_y, data.mag_z);
    
    return data;
}

bool ICM20948Driver::readMagnetometer(float& x, float& y, float& z) {
    // Simplified magnetometer read (requires I2C master setup)
    // Full implementation would enable I2C master and read from AK09916
    x = y = z = 0.0f;
    return false;
}

CapabilitySchema ICM20948Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings
    schema.addSetting("driver", ParamType::STRING, true, "icm20948", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_ICM20948_TIER_NAME, "", "", "");
    
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

String ICM20948Driver::getParameter(const String& name) {
    return "";
}

bool ICM20948Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_ICM20948_ENABLE_CONFIGURATION
    if (name == "accel_range") {
        return setAccelRange(value.toInt());
    } else if (name == "gyro_range") {
        return setGyroRange(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_ICM20948_ENABLE_CONFIGURATION
bool ICM20948Driver::setAccelRange(uint8_t range) {
    if (!initialized) return false;
    selectBank(2);
    bool result = writeRegister(ICM20948_REG_ACCEL_CONFIG, range);
    selectBank(0);
    
    // Update scale factor
    float ranges[] = {2.0f, 4.0f, 8.0f, 16.0f};
    if (range < 4) {
        accelScale = ranges[range] / 32768.0f * 9.81f;
    }
    
    return result;
}

bool ICM20948Driver::setGyroRange(uint8_t range) {
    if (!initialized) return false;
    selectBank(2);
    bool result = writeRegister(ICM20948_REG_GYRO_CONFIG_1, range);
    selectBank(0);
    
    // Update scale factor
    float ranges[] = {250.0f, 500.0f, 1000.0f, 2000.0f};
    if (range < 4) {
        gyroScale = ranges[range] / 32768.0f * 0.017453293f;
    }
    
    return result;
}
#endif

#if POCKETOS_ICM20948_ENABLE_REGISTER_ACCESS
const RegisterDesc* ICM20948Driver::registers(size_t& count) const {
    count = ICM20948_REGISTER_COUNT;
    return ICM20948_REGISTERS;
}

bool ICM20948Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        ICM20948_REGISTERS, ICM20948_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool ICM20948Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        ICM20948_REGISTERS, ICM20948_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* ICM20948Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(ICM20948_REGISTERS, ICM20948_REGISTER_COUNT, name);
}
#endif

bool ICM20948Driver::selectBank(uint8_t bank) {
    return writeRegister(ICM20948_REG_REG_BANK_SEL, bank << 4);
}

bool ICM20948Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool ICM20948Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool ICM20948Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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
