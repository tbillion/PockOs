#include "fxas21002c_driver.h"
#include "../driver_config.h"

#if POCKETOS_FXAS21002C_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// FXAS21002C Register addresses
#define FXAS21002C_REG_STATUS       0x00
#define FXAS21002C_REG_OUT_X_MSB    0x01
#define FXAS21002C_REG_OUT_X_LSB    0x02
#define FXAS21002C_REG_OUT_Y_MSB    0x03
#define FXAS21002C_REG_OUT_Y_LSB    0x04
#define FXAS21002C_REG_OUT_Z_MSB    0x05
#define FXAS21002C_REG_OUT_Z_LSB    0x06
#define FXAS21002C_REG_WHO_AM_I     0x0C
#define FXAS21002C_REG_CTRL_REG0    0x0D
#define FXAS21002C_REG_CTRL_REG1    0x13
#define FXAS21002C_REG_TEMP         0x12

// WHO_AM_I value
#define FXAS21002C_WHO_AM_I_VALUE   0xD7

#if POCKETOS_FXAS21002C_ENABLE_REGISTER_ACCESS
static const RegisterDesc FXAS21002C_REGISTERS[] = {
    RegisterDesc(0x00, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "OUT_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "OUT_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x03, "OUT_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x04, "OUT_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x05, "OUT_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x06, "OUT_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "WHO_AM_I", 1, RegisterAccess::RO, 0xD7),
    RegisterDesc(0x0D, "CTRL_REG0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "TEMP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x13, "CTRL_REG1", 1, RegisterAccess::RW, 0x00),
};
#define FXAS21002C_REGISTER_COUNT (sizeof(FXAS21002C_REGISTERS) / sizeof(RegisterDesc))
#endif

FXAS21002CDriver::FXAS21002CDriver() : address(0), initialized(false), gyroScale(1.0f) {}

bool FXAS21002CDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_FXAS21002C_ENABLE_LOGGING
    Logger::info("FXAS21002C: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check WHO_AM_I
    uint8_t whoAmI = 0;
    if (!readRegister(FXAS21002C_REG_WHO_AM_I, &whoAmI)) {
#if POCKETOS_FXAS21002C_ENABLE_LOGGING
        Logger::error("FXAS21002C: Failed to read WHO_AM_I");
#endif
        return false;
    }
    
    if (whoAmI != FXAS21002C_WHO_AM_I_VALUE) {
#if POCKETOS_FXAS21002C_ENABLE_LOGGING
        Logger::error("FXAS21002C: Invalid WHO_AM_I: 0x" + String(whoAmI, HEX));
#endif
        return false;
    }
    
#if POCKETOS_FXAS21002C_ENABLE_CONFIGURATION
    // Put device in standby mode
    writeRegister(FXAS21002C_REG_CTRL_REG1, 0x00);
    delay(10);
    
    // Configure CTRL_REG0: ±250 dps range
    writeRegister(FXAS21002C_REG_CTRL_REG0, 0x03);
    gyroScale = 250.0f / 32768.0f * 0.017453293f;  // dps to rad/s
    
    // Activate device: 100Hz ODR, Active mode
    writeRegister(FXAS21002C_REG_CTRL_REG1, 0x0E);
    delay(100);
#else
    // Minimal: activate with default settings
    writeRegister(FXAS21002C_REG_CTRL_REG1, 0x0E);
    delay(100);
    gyroScale = 250.0f / 32768.0f * 0.017453293f;  // Default ±250 dps to rad/s
#endif
    
    initialized = true;
#if POCKETOS_FXAS21002C_ENABLE_LOGGING
    Logger::info("FXAS21002C: Initialized successfully");
#endif
    return true;
}

void FXAS21002CDriver::deinit() {
    if (initialized) {
        writeRegister(FXAS21002C_REG_CTRL_REG1, 0x00);  // Standby
    }
    initialized = false;
}

FXAS21002CData FXAS21002CDriver::readData() {
    FXAS21002CData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read gyroscope data (6 bytes)
    if (readRegisters(FXAS21002C_REG_OUT_X_MSB, buffer, 6)) {
        int16_t x = (int16_t)((buffer[0] << 8) | buffer[1]);
        int16_t y = (int16_t)((buffer[2] << 8) | buffer[3]);
        int16_t z = (int16_t)((buffer[4] << 8) | buffer[5]);
        
        data.gyro_x = x * gyroScale;
        data.gyro_y = y * gyroScale;
        data.gyro_z = z * gyroScale;
    }
    
    // Read temperature
    uint8_t temp;
    if (readRegister(FXAS21002C_REG_TEMP, &temp)) {
        data.temperature = (int8_t)temp;  // Already in °C
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema FXAS21002CDriver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings
    schema.addSetting("driver", ParamType::STRING, false, 0, 0, 0, "");
    schema.addSetting("tier", ParamType::STRING, false, 0, 0, 0, "");
    
    // Output signals
    schema.addSignal("gyro_x", ParamType::FLOAT, false, "rad/s");
    schema.addSignal("gyro_y", ParamType::FLOAT, false, "rad/s");
    schema.addSignal("gyro_z", ParamType::FLOAT, false, "rad/s");
    schema.addSignal("temperature", ParamType::FLOAT, false, "°C");
    
    return schema;
}

String FXAS21002CDriver::getParameter(const String& name) {
#if POCKETOS_FXAS21002C_ENABLE_CONFIGURATION
    if (name == "gyro_range") {
        uint8_t ctrl0;
        if (readRegister(FXAS21002C_REG_CTRL_REG0, &ctrl0)) {
            return String(ctrl0 & 0x03);
        }
    }
#endif
    return "";
}

bool FXAS21002CDriver::setParameter(const String& name, const String& value) {
#if POCKETOS_FXAS21002C_ENABLE_CONFIGURATION
    if (name == "gyro_range") {
        return setGyroRange(value.toInt());
    } else if (name == "odr") {
        return setOutputDataRate(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_FXAS21002C_ENABLE_CONFIGURATION
bool FXAS21002CDriver::setGyroRange(uint8_t range) {
    if (!initialized || range > 3) return false;
    
    // Put in standby
    writeRegister(FXAS21002C_REG_CTRL_REG1, 0x00);
    delay(10);
    
    // Set range
    writeRegister(FXAS21002C_REG_CTRL_REG0, range);
    
    // Update scale factor
    float ranges[] = { 2000.0f, 1000.0f, 500.0f, 250.0f };
    gyroScale = ranges[range] / 32768.0f * 0.017453293f;
    
    // Reactivate
    writeRegister(FXAS21002C_REG_CTRL_REG1, 0x0E);
    delay(100);
    
    return true;
}

bool FXAS21002CDriver::setOutputDataRate(uint8_t odr) {
    if (!initialized || odr > 7) return false;
    
    uint8_t ctrl1;
    if (!readRegister(FXAS21002C_REG_CTRL_REG1, &ctrl1)) {
        return false;
    }
    
    // Clear ODR bits and set new value
    ctrl1 = (ctrl1 & 0xE3) | ((odr & 0x07) << 2);
    writeRegister(FXAS21002C_REG_CTRL_REG1, ctrl1);
    delay(10);
    
    return true;
}
#endif

#if POCKETOS_FXAS21002C_ENABLE_REGISTER_ACCESS
const RegisterDesc* FXAS21002CDriver::registers(size_t& count) const {
    count = FXAS21002C_REGISTER_COUNT;
    return FXAS21002C_REGISTERS;
}

bool FXAS21002CDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        FXAS21002C_REGISTERS, FXAS21002C_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool FXAS21002CDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        FXAS21002C_REGISTERS, FXAS21002C_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* FXAS21002CDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(FXAS21002C_REGISTERS, FXAS21002C_REGISTER_COUNT, name);
}
#endif

bool FXAS21002CDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool FXAS21002CDriver::readRegister(uint8_t reg, uint8_t* value) {
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

bool FXAS21002CDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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
