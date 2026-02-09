#include "lsm303agr_driver.h"
#include "../driver_config.h"

#if POCKETOS_LSM303AGR_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// LSM303AGR Accelerometer Register addresses
#define LSM303AGR_ACCEL_WHO_AM_I    0x0F
#define LSM303AGR_ACCEL_CTRL_REG1   0x20
#define LSM303AGR_ACCEL_CTRL_REG4   0x23
#define LSM303AGR_ACCEL_OUT_X_L     0x28
#define LSM303AGR_ACCEL_TEMP_OUT_L  0x0C

// LSM303AGR Magnetometer Register addresses
#define LSM303AGR_MAG_WHO_AM_I      0x4F
#define LSM303AGR_MAG_CFG_REG_A     0x60
#define LSM303AGR_MAG_CFG_REG_C     0x62
#define LSM303AGR_MAG_OUT_X_L       0x68

// WHO_AM_I values
#define LSM303AGR_ACCEL_WHO_AM_I_VALUE  0x33
#define LSM303AGR_MAG_WHO_AM_I_VALUE    0x40

#if POCKETOS_LSM303AGR_ENABLE_REGISTER_ACCESS
static const RegisterDesc LSM303AGR_REGISTERS[] = {
    // Accelerometer registers
    RegisterDesc(0x0F, "ACCEL_WHO_AM_I", 1, RegisterAccess::RO, 0x33),
    RegisterDesc(0x20, "ACCEL_CTRL_REG1", 1, RegisterAccess::RW, 0x07),
    RegisterDesc(0x21, "ACCEL_CTRL_REG2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "ACCEL_CTRL_REG3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x23, "ACCEL_CTRL_REG4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "ACCEL_OUT_X_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "ACCEL_OUT_X_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "ACCEL_OUT_Y_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "ACCEL_OUT_Y_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "ACCEL_OUT_Z_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2D, "ACCEL_OUT_Z_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "ACCEL_TEMP_OUT_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "ACCEL_TEMP_OUT_H", 1, RegisterAccess::RO, 0x00),
    // Magnetometer registers
    RegisterDesc(0x4F, "MAG_WHO_AM_I", 1, RegisterAccess::RO, 0x40),
    RegisterDesc(0x60, "MAG_CFG_REG_A", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x61, "MAG_CFG_REG_B", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x62, "MAG_CFG_REG_C", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x68, "MAG_OUT_X_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x69, "MAG_OUT_X_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6A, "MAG_OUT_Y_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6B, "MAG_OUT_Y_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6C, "MAG_OUT_Z_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6D, "MAG_OUT_Z_H", 1, RegisterAccess::RO, 0x00),
};
#define LSM303AGR_REGISTER_COUNT (sizeof(LSM303AGR_REGISTERS) / sizeof(RegisterDesc))
#endif

LSM303AGRDriver::LSM303AGRDriver() : accelAddr(0), magAddr(0), initialized(false), 
                                      accelScale(0.001f), magScale(1.5f) {}

bool LSM303AGRDriver::init(uint8_t i2cAddress) {
    // The LSM303AGR has two I2C addresses: accel at 0x19, mag at 0x1E
    // The i2cAddress parameter can be either one - we'll detect and use both
    accelAddr = 0x19;
    magAddr = 0x1E;
    
#if POCKETOS_LSM303AGR_ENABLE_LOGGING
    Logger::info("LSM303AGR: Initializing dual-chip sensor");
#endif
    
    // Check accelerometer WHO_AM_I
    uint8_t accelWhoami;
    if (!readRegister(accelAddr, LSM303AGR_ACCEL_WHO_AM_I, &accelWhoami)) {
#if POCKETOS_LSM303AGR_ENABLE_LOGGING
        Logger::error("LSM303AGR: Failed to read accel WHO_AM_I at 0x" + String(accelAddr, HEX));
#endif
        return false;
    }
    
    if (accelWhoami != LSM303AGR_ACCEL_WHO_AM_I_VALUE) {
#if POCKETOS_LSM303AGR_ENABLE_LOGGING
        Logger::error("LSM303AGR: Invalid accel WHO_AM_I: 0x" + String(accelWhoami, HEX));
#endif
        return false;
    }
    
    // Check magnetometer WHO_AM_I
    uint8_t magWhoami;
    if (!readRegister(magAddr, LSM303AGR_MAG_WHO_AM_I, &magWhoami)) {
#if POCKETOS_LSM303AGR_ENABLE_LOGGING
        Logger::error("LSM303AGR: Failed to read mag WHO_AM_I at 0x" + String(magAddr, HEX));
#endif
        return false;
    }
    
    if (magWhoami != LSM303AGR_MAG_WHO_AM_I_VALUE) {
#if POCKETOS_LSM303AGR_ENABLE_LOGGING
        Logger::error("LSM303AGR: Invalid mag WHO_AM_I: 0x" + String(magWhoami, HEX));
#endif
        return false;
    }
    
#if POCKETOS_LSM303AGR_ENABLE_CONFIGURATION
    // Configure accelerometer: 100Hz, normal mode, all axes enabled
    writeRegister(accelAddr, LSM303AGR_ACCEL_CTRL_REG1, 0x57);
    
    // Set accel scale to ±2g (default)
    writeRegister(accelAddr, LSM303AGR_ACCEL_CTRL_REG4, 0x00);
    accelScale = 0.001f;  // 1 mg/LSB
    
    // Configure magnetometer: continuous mode, 10Hz
    writeRegister(magAddr, LSM303AGR_MAG_CFG_REG_A, 0x00);
    writeRegister(magAddr, LSM303AGR_MAG_CFG_REG_C, 0x00);
    magScale = 1.5f;  // 1.5 µT/LSB
#else
    // Minimal configuration
    writeRegister(accelAddr, LSM303AGR_ACCEL_CTRL_REG1, 0x57);
    writeRegister(accelAddr, LSM303AGR_ACCEL_CTRL_REG4, 0x00);
    writeRegister(magAddr, LSM303AGR_MAG_CFG_REG_A, 0x00);
    writeRegister(magAddr, LSM303AGR_MAG_CFG_REG_C, 0x00);
    accelScale = 0.001f;
    magScale = 1.5f;
#endif
    
    delay(10);  // Wait for first measurement
    
    initialized = true;
#if POCKETOS_LSM303AGR_ENABLE_LOGGING
    Logger::info("LSM303AGR: Initialized successfully");
#endif
    return true;
}

void LSM303AGRDriver::deinit() {
    if (initialized) {
        writeRegister(accelAddr, LSM303AGR_ACCEL_CTRL_REG1, 0x00);  // Power down accel
        writeRegister(magAddr, LSM303AGR_MAG_CFG_REG_A, 0x03);     // Idle mode mag
    }
    initialized = false;
}

LSM303AGRData LSM303AGRDriver::readData() {
    LSM303AGRData data;
    
    if (!initialized) {
        return data;
    }
    
    // Read accelerometer data (6 bytes)
    uint8_t accelBuffer[6];
    if (readRegisters(accelAddr, LSM303AGR_ACCEL_OUT_X_L | 0x80, accelBuffer, 6)) {
        int16_t ax = (int16_t)((accelBuffer[1] << 8) | accelBuffer[0]);
        int16_t ay = (int16_t)((accelBuffer[3] << 8) | accelBuffer[2]);
        int16_t az = (int16_t)((accelBuffer[5] << 8) | accelBuffer[4]);
        
        // Convert to m/s² (accelScale is in g/LSB, multiply by 9.81)
        data.accel_x = (ax >> 4) * accelScale * 9.81f;
        data.accel_y = (ay >> 4) * accelScale * 9.81f;
        data.accel_z = (az >> 4) * accelScale * 9.81f;
    }
    
    // Read magnetometer data (6 bytes)
    uint8_t magBuffer[6];
    if (readRegisters(magAddr, LSM303AGR_MAG_OUT_X_L | 0x80, magBuffer, 6)) {
        int16_t mx = (int16_t)((magBuffer[1] << 8) | magBuffer[0]);
        int16_t my = (int16_t)((magBuffer[3] << 8) | magBuffer[2]);
        int16_t mz = (int16_t)((magBuffer[5] << 8) | magBuffer[4]);
        
        data.mag_x = mx * magScale;
        data.mag_y = my * magScale;
        data.mag_z = mz * magScale;
    }
    
    // Read temperature from accelerometer (2 bytes)
    uint8_t tempBuffer[2];
    if (readRegisters(accelAddr, LSM303AGR_ACCEL_TEMP_OUT_L | 0x80, tempBuffer, 2)) {
        int16_t temp = (int16_t)((tempBuffer[1] << 8) | tempBuffer[0]);
        data.temperature = 25.0f + (temp >> 8);  // Temperature offset from 25°C
    }
    
    data.valid = true;
    
    return data;
}

CapabilitySchema LSM303AGRDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_LSM303AGR_TIER_NAME;
    
    schema.addOutput("accel_x", "number", "m/s²", "X-axis acceleration");
    schema.addOutput("accel_y", "number", "m/s²", "Y-axis acceleration");
    schema.addOutput("accel_z", "number", "m/s²", "Z-axis acceleration");
    schema.addOutput("mag_x", "number", "µT", "X-axis magnetic field");
    schema.addOutput("mag_y", "number", "µT", "Y-axis magnetic field");
    schema.addOutput("mag_z", "number", "µT", "Z-axis magnetic field");
    schema.addOutput("temperature", "number", "°C", "Temperature");
    
    return schema;
}

String LSM303AGRDriver::getParameter(const String& name) {
    return "";
}

bool LSM303AGRDriver::setParameter(const String& name, const String& value) {
#if POCKETOS_LSM303AGR_ENABLE_CONFIGURATION
    if (name == "accel_data_rate") {
        return setAccelDataRate(value.toInt());
    } else if (name == "accel_scale") {
        return setAccelScale(value.toInt());
    } else if (name == "mag_data_rate") {
        return setMagDataRate(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_LSM303AGR_ENABLE_CONFIGURATION
bool LSM303AGRDriver::setAccelDataRate(uint8_t rate) {
    if (!initialized || rate > 9) return false;
    
    uint8_t ctrl1;
    if (!readRegister(accelAddr, LSM303AGR_ACCEL_CTRL_REG1, &ctrl1)) return false;
    
    ctrl1 = (ctrl1 & 0x0F) | (rate << 4);
    return writeRegister(accelAddr, LSM303AGR_ACCEL_CTRL_REG1, ctrl1);
}

bool LSM303AGRDriver::setAccelScale(uint8_t scale) {
    if (!initialized || scale > 3) return false;
    
    // Update scale factor: 0=±2g, 1=±4g, 2=±8g, 3=±16g
    const float scales[] = {0.001f, 0.002f, 0.004f, 0.012f};
    accelScale = scales[scale];
    
    return true;
}

bool LSM303AGRDriver::setMagDataRate(uint8_t rate) {
    if (!initialized || rate > 3) return false;
    
    uint8_t cfgA;
    if (!readRegister(magAddr, LSM303AGR_MAG_CFG_REG_A, &cfgA)) return false;
    
    cfgA = (cfgA & 0xF3) | (rate << 2);
    return writeRegister(magAddr, LSM303AGR_MAG_CFG_REG_A, cfgA);
}
#endif

#if POCKETOS_LSM303AGR_ENABLE_REGISTER_ACCESS
const RegisterDesc* LSM303AGRDriver::registers(size_t& count) const {
    count = LSM303AGR_REGISTER_COUNT;
    return LSM303AGR_REGISTERS;
}

bool LSM303AGRDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LSM303AGR_REGISTERS, LSM303AGR_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    // Determine which address to use based on register
    uint8_t addr = (reg >= 0x60) ? magAddr : accelAddr;
    return readRegister(addr, (uint8_t)reg, buf);
}

bool LSM303AGRDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        LSM303AGR_REGISTERS, LSM303AGR_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    // Determine which address to use based on register
    uint8_t addr = (reg >= 0x60) ? magAddr : accelAddr;
    return writeRegister(addr, (uint8_t)reg, buf[0]);
}

const RegisterDesc* LSM303AGRDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(LSM303AGR_REGISTERS, LSM303AGR_REGISTER_COUNT, name);
}
#endif

bool LSM303AGRDriver::writeRegister(uint8_t addr, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool LSM303AGRDriver::readRegister(uint8_t addr, uint8_t reg, uint8_t* value) {
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

bool LSM303AGRDriver::readRegisters(uint8_t addr, uint8_t reg, uint8_t* buffer, size_t len) {
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
