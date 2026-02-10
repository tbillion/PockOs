#include "hmc5883l_driver.h"
#include "../driver_config.h"

#if POCKETOS_HMC5883L_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// HMC5883L Register addresses
#define HMC5883L_REG_CONFIG_A      0x00
#define HMC5883L_REG_CONFIG_B      0x01
#define HMC5883L_REG_MODE          0x02
#define HMC5883L_REG_DATA_X_MSB    0x03
#define HMC5883L_REG_ID_A          0x0A
#define HMC5883L_REG_ID_B          0x0B
#define HMC5883L_REG_ID_C          0x0C

// ID values
#define HMC5883L_ID_A_VALUE        0x48  // 'H'
#define HMC5883L_ID_B_VALUE        0x34  // '4'
#define HMC5883L_ID_C_VALUE        0x33  // '3'

#if POCKETOS_HMC5883L_ENABLE_REGISTER_ACCESS
static const RegisterDesc HMC5883L_REGISTERS[] = {
    RegisterDesc(0x00, "CONFIG_A", 1, RegisterAccess::RW, 0x10),
    RegisterDesc(0x01, "CONFIG_B", 1, RegisterAccess::RW, 0x20),
    RegisterDesc(0x02, "MODE", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x03, "DATA_X_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x04, "DATA_X_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x05, "DATA_Z_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x06, "DATA_Z_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x07, "DATA_Y_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x08, "DATA_Y_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "ID_A", 1, RegisterAccess::RO, 0x48),
    RegisterDesc(0x0B, "ID_B", 1, RegisterAccess::RO, 0x34),
    RegisterDesc(0x0C, "ID_C", 1, RegisterAccess::RO, 0x33),
};
#define HMC5883L_REGISTER_COUNT (sizeof(HMC5883L_REGISTERS) / sizeof(RegisterDesc))
#endif

HMC5883LDriver::HMC5883LDriver() : address(0), initialized(false), magGain(1.0f) {}

bool HMC5883LDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_HMC5883L_ENABLE_LOGGING
    Logger::info("HMC5883L: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check ID registers
    uint8_t idA, idB, idC;
    if (!readRegister(HMC5883L_REG_ID_A, &idA) ||
        !readRegister(HMC5883L_REG_ID_B, &idB) ||
        !readRegister(HMC5883L_REG_ID_C, &idC)) {
#if POCKETOS_HMC5883L_ENABLE_LOGGING
        Logger::error("HMC5883L: Failed to read ID registers");
#endif
        return false;
    }
    
    if (idA != HMC5883L_ID_A_VALUE || idB != HMC5883L_ID_B_VALUE || idC != HMC5883L_ID_C_VALUE) {
#if POCKETOS_HMC5883L_ENABLE_LOGGING
        Logger::error("HMC5883L: Invalid ID values");
#endif
        return false;
    }
    
#if POCKETOS_HMC5883L_ENABLE_CONFIGURATION
    // Configure: 8 samples average, 15Hz output rate, normal measurement
    writeRegister(HMC5883L_REG_CONFIG_A, 0x70);
    
    // Set gain to ±1.3 Ga (default, LSB = 0.92 mG)
    writeRegister(HMC5883L_REG_CONFIG_B, 0x20);
    magGain = 0.92f / 10.0f;  // Convert to µT
    
    // Continuous measurement mode
    writeRegister(HMC5883L_REG_MODE, 0x00);
#else
    // Minimal: continuous mode
    writeRegister(HMC5883L_REG_MODE, 0x00);
    magGain = 0.92f / 10.0f;  // Default gain
#endif
    
    delay(67);  // Wait for first measurement
    
    initialized = true;
#if POCKETOS_HMC5883L_ENABLE_LOGGING
    Logger::info("HMC5883L: Initialized successfully");
#endif
    return true;
}

void HMC5883LDriver::deinit() {
    if (initialized) {
        writeRegister(HMC5883L_REG_MODE, 0x03);  // Sleep mode
    }
    initialized = false;
}

HMC5883LData HMC5883LDriver::readData() {
    HMC5883LData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[6];
    
    // Read 6 bytes: X_MSB, X_LSB, Z_MSB, Z_LSB, Y_MSB, Y_LSB (note: Z before Y!)
    if (readRegisters(HMC5883L_REG_DATA_X_MSB, buffer, 6)) {
        int16_t x = (int16_t)((buffer[0] << 8) | buffer[1]);
        int16_t z = (int16_t)((buffer[2] << 8) | buffer[3]);
        int16_t y = (int16_t)((buffer[4] << 8) | buffer[5]);
        
        data.mag_x = x * magGain;
        data.mag_y = y * magGain;
        data.mag_z = z * magGain;
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema HMC5883LDriver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_HMC5883L_TIER_NAME;
    
    schema.addOutput("mag_x", "number", "µT", "X-axis magnetic field");
    schema.addOutput("mag_y", "number", "µT", "Y-axis magnetic field");
    schema.addOutput("mag_z", "number", "µT", "Z-axis magnetic field");
    
    return schema;
}

String HMC5883LDriver::getParameter(const String& name) {
    return "";
}

bool HMC5883LDriver::setParameter(const String& name, const String& value) {
#if POCKETOS_HMC5883L_ENABLE_CONFIGURATION
    if (name == "gain") {
        return setGain(value.toInt());
    } else if (name == "mode") {
        return setMode(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_HMC5883L_ENABLE_CONFIGURATION
bool HMC5883LDriver::setGain(uint8_t gain) {
    if (!initialized || gain > 7) return false;
    
    writeRegister(HMC5883L_REG_CONFIG_B, gain << 5);
    
    // Update gain scale
    const float gains[] = {0.73f, 0.92f, 1.22f, 1.52f, 2.27f, 2.56f, 3.03f, 4.35f};
    magGain = gains[gain] / 10.0f;  // Convert to µT
    
    return true;
}

bool HMC5883LDriver::setMode(uint8_t mode) {
    if (!initialized || mode > 3) return false;
    return writeRegister(HMC5883L_REG_MODE, mode);
}
#endif

#if POCKETOS_HMC5883L_ENABLE_REGISTER_ACCESS
const RegisterDesc* HMC5883LDriver::registers(size_t& count) const {
    count = HMC5883L_REGISTER_COUNT;
    return HMC5883L_REGISTERS;
}

bool HMC5883LDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        HMC5883L_REGISTERS, HMC5883L_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool HMC5883LDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        HMC5883L_REGISTERS, HMC5883L_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* HMC5883LDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(HMC5883L_REGISTERS, HMC5883L_REGISTER_COUNT, name);
}
#endif

bool HMC5883LDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool HMC5883LDriver::readRegister(uint8_t reg, uint8_t* value) {
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

bool HMC5883LDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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
