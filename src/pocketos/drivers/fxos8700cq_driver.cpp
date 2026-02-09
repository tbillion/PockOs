#include "fxos8700cq_driver.h"
#include "../driver_config.h"

#if POCKETOS_FXOS8700CQ_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// FXOS8700CQ Register addresses
#define FXOS8700CQ_REG_STATUS       0x00
#define FXOS8700CQ_REG_OUT_X_MSB    0x01
#define FXOS8700CQ_REG_OUT_X_LSB    0x02
#define FXOS8700CQ_REG_OUT_Y_MSB    0x03
#define FXOS8700CQ_REG_OUT_Y_LSB    0x04
#define FXOS8700CQ_REG_OUT_Z_MSB    0x05
#define FXOS8700CQ_REG_OUT_Z_LSB    0x06
#define FXOS8700CQ_REG_WHO_AM_I     0x0D
#define FXOS8700CQ_REG_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_REG_CTRL_REG1    0x2A
#define FXOS8700CQ_REG_M_OUT_X_MSB  0x33
#define FXOS8700CQ_REG_M_OUT_X_LSB  0x34
#define FXOS8700CQ_REG_M_OUT_Y_MSB  0x35
#define FXOS8700CQ_REG_M_OUT_Y_LSB  0x36
#define FXOS8700CQ_REG_M_OUT_Z_MSB  0x37
#define FXOS8700CQ_REG_M_OUT_Z_LSB  0x38
#define FXOS8700CQ_REG_TEMP         0x51
#define FXOS8700CQ_REG_M_CTRL_REG1  0x5B
#define FXOS8700CQ_REG_M_CTRL_REG2  0x5C

// WHO_AM_I value
#define FXOS8700CQ_WHO_AM_I_VALUE   0xC7

#if POCKETOS_FXOS8700CQ_ENABLE_REGISTER_ACCESS
static const RegisterDesc FXOS8700CQ_REGISTERS[] = {
    RegisterDesc(0x00, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "OUT_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "OUT_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x03, "OUT_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x04, "OUT_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x05, "OUT_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x06, "OUT_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "WHO_AM_I", 1, RegisterAccess::RO, 0xC7),
    RegisterDesc(0x0E, "XYZ_DATA_CFG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2A, "CTRL_REG1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x33, "M_OUT_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x34, "M_OUT_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x35, "M_OUT_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x36, "M_OUT_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x37, "M_OUT_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x38, "M_OUT_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x51, "TEMP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x5B, "M_CTRL_REG1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5C, "M_CTRL_REG2", 1, RegisterAccess::RW, 0x00),
};
#define FXOS8700CQ_REGISTER_COUNT (sizeof(FXOS8700CQ_REGISTERS) / sizeof(RegisterDesc))
#endif

FXOS8700CQDriver::FXOS8700CQDriver() : address(0), initialized(false), accelScale(1.0f) {}

bool FXOS8700CQDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_FXOS8700CQ_ENABLE_LOGGING
    Logger::info("FXOS8700CQ: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check WHO_AM_I
    uint8_t whoAmI = 0;
    if (!readRegister(FXOS8700CQ_REG_WHO_AM_I, &whoAmI)) {
#if POCKETOS_FXOS8700CQ_ENABLE_LOGGING
        Logger::error("FXOS8700CQ: Failed to read WHO_AM_I");
#endif
        return false;
    }
    
    if (whoAmI != FXOS8700CQ_WHO_AM_I_VALUE) {
#if POCKETOS_FXOS8700CQ_ENABLE_LOGGING
        Logger::error("FXOS8700CQ: Invalid WHO_AM_I: 0x" + String(whoAmI, HEX));
#endif
        return false;
    }
    
#if POCKETOS_FXOS8700CQ_ENABLE_CONFIGURATION
    // Put device in standby mode
    writeRegister(FXOS8700CQ_REG_CTRL_REG1, 0x00);
    delay(10);
    
    // Configure ±2g range
    writeRegister(FXOS8700CQ_REG_XYZ_DATA_CFG, 0x00);
    accelScale = 2.0f / 32768.0f * 9.81f;  // ±2g to m/s²
    
    // Enable hybrid mode (accel + mag)
    writeRegister(FXOS8700CQ_REG_M_CTRL_REG1, 0x1F);
    writeRegister(FXOS8700CQ_REG_M_CTRL_REG2, 0x20);
    
    // Activate device: 100Hz ODR, Hybrid mode active
    writeRegister(FXOS8700CQ_REG_CTRL_REG1, 0x15);
    delay(100);
#else
    // Minimal: activate with default settings
    writeRegister(FXOS8700CQ_REG_M_CTRL_REG1, 0x1F);
    writeRegister(FXOS8700CQ_REG_M_CTRL_REG2, 0x20);
    writeRegister(FXOS8700CQ_REG_CTRL_REG1, 0x15);
    delay(100);
    accelScale = 2.0f / 32768.0f * 9.81f;  // Default ±2g to m/s²
#endif
    
    initialized = true;
#if POCKETOS_FXOS8700CQ_ENABLE_LOGGING
    Logger::info("FXOS8700CQ: Initialized successfully");
#endif
    return true;
}

void FXOS8700CQDriver::deinit() {
    if (initialized) {
        writeRegister(FXOS8700CQ_REG_CTRL_REG1, 0x00);  // Standby
    }
    initialized = false;
}

FXOS8700CQData FXOS8700CQDriver::readData() {
    FXOS8700CQData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read accelerometer data (6 bytes)
    if (readRegisters(FXOS8700CQ_REG_OUT_X_MSB, buffer, 6)) {
        int16_t x = (int16_t)((buffer[0] << 8) | buffer[1]);
        int16_t y = (int16_t)((buffer[2] << 8) | buffer[3]);
        int16_t z = (int16_t)((buffer[4] << 8) | buffer[5]);
        
        data.accel_x = (x >> 2) * accelScale;
        data.accel_y = (y >> 2) * accelScale;
        data.accel_z = (z >> 2) * accelScale;
    }
    
    // Read magnetometer data (6 bytes)
    if (readRegisters(FXOS8700CQ_REG_M_OUT_X_MSB, buffer, 6)) {
        int16_t x = (int16_t)((buffer[0] << 8) | buffer[1]);
        int16_t y = (int16_t)((buffer[2] << 8) | buffer[3]);
        int16_t z = (int16_t)((buffer[4] << 8) | buffer[5]);
        
        data.mag_x = x * 0.1f;  // Convert to µT
        data.mag_y = y * 0.1f;
        data.mag_z = z * 0.1f;
    }
    
    // Read temperature
    uint8_t temp;
    if (readRegister(FXOS8700CQ_REG_TEMP, &temp)) {
        data.temperature = (int8_t)temp * 0.96f;  // 0.96°C per LSB
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema FXOS8700CQDriver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings
    schema.addSetting("driver", ParamType::STRING, false, 0, 0, 0, "");
    schema.addSetting("tier", ParamType::STRING, false, 0, 0, 0, "");
    
    // Output signals
    schema.addSignal("accel_x", ParamType::FLOAT, false, "m/s²");
    schema.addSignal("accel_y", ParamType::FLOAT, false, "m/s²");
    schema.addSignal("accel_z", ParamType::FLOAT, false, "m/s²");
    schema.addSignal("mag_x", ParamType::FLOAT, false, "µT");
    schema.addSignal("mag_y", ParamType::FLOAT, false, "µT");
    schema.addSignal("mag_z", ParamType::FLOAT, false, "µT");
    schema.addSignal("temperature", ParamType::FLOAT, false, "°C");
    
    return schema;
}

String FXOS8700CQDriver::getParameter(const String& name) {
#if POCKETOS_FXOS8700CQ_ENABLE_CONFIGURATION
    if (name == "accel_range") {
        uint8_t cfg;
        if (readRegister(FXOS8700CQ_REG_XYZ_DATA_CFG, &cfg)) {
            return String(cfg & 0x03);
        }
    }
#endif
    return "";
}

bool FXOS8700CQDriver::setParameter(const String& name, const String& value) {
#if POCKETOS_FXOS8700CQ_ENABLE_CONFIGURATION
    if (name == "accel_range") {
        return setAccelRange(value.toInt());
    } else if (name == "hybrid_mode") {
        return setHybridMode(value.toInt() != 0);
    }
#endif
    return false;
}

#if POCKETOS_FXOS8700CQ_ENABLE_CONFIGURATION
bool FXOS8700CQDriver::setAccelRange(uint8_t range) {
    if (!initialized || range > 2) return false;
    
    // Put in standby
    writeRegister(FXOS8700CQ_REG_CTRL_REG1, 0x00);
    delay(10);
    
    // Set range
    writeRegister(FXOS8700CQ_REG_XYZ_DATA_CFG, range);
    
    // Update scale factor
    float ranges[] = { 2.0f, 4.0f, 8.0f };
    accelScale = ranges[range] / 32768.0f * 9.81f;
    
    // Reactivate
    writeRegister(FXOS8700CQ_REG_CTRL_REG1, 0x15);
    delay(100);
    
    return true;
}

bool FXOS8700CQDriver::setHybridMode(bool enable) {
    if (!initialized) return false;
    
    uint8_t ctrl1;
    if (!readRegister(FXOS8700CQ_REG_CTRL_REG1, &ctrl1)) {
        return false;
    }
    
    // Put in standby
    writeRegister(FXOS8700CQ_REG_CTRL_REG1, 0x00);
    delay(10);
    
    if (enable) {
        writeRegister(FXOS8700CQ_REG_M_CTRL_REG1, 0x1F);
        writeRegister(FXOS8700CQ_REG_M_CTRL_REG2, 0x20);
        ctrl1 |= 0x04;  // Set hybrid mode bit
    } else {
        ctrl1 &= ~0x04;  // Clear hybrid mode bit
    }
    
    // Reactivate
    writeRegister(FXOS8700CQ_REG_CTRL_REG1, ctrl1);
    delay(100);
    
    return true;
}
#endif

#if POCKETOS_FXOS8700CQ_ENABLE_REGISTER_ACCESS
const RegisterDesc* FXOS8700CQDriver::registers(size_t& count) const {
    count = FXOS8700CQ_REGISTER_COUNT;
    return FXOS8700CQ_REGISTERS;
}

bool FXOS8700CQDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        FXOS8700CQ_REGISTERS, FXOS8700CQ_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool FXOS8700CQDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        FXOS8700CQ_REGISTERS, FXOS8700CQ_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* FXOS8700CQDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(FXOS8700CQ_REGISTERS, FXOS8700CQ_REGISTER_COUNT, name);
}
#endif

bool FXOS8700CQDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool FXOS8700CQDriver::readRegister(uint8_t reg, uint8_t* value) {
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

bool FXOS8700CQDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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
