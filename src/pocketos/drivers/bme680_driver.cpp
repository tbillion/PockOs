#include "bme680_driver.h"
#include "../driver_config.h"
#if POCKETOS_BME680_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define BME680_REG_CHIP_ID      0xD0
#define BME680_REG_VARIANT_ID   0xF0
#define BME680_REG_RESET        0xE0
#define BME680_REG_CTRL_GAS_1   0x71
#define BME680_REG_CTRL_HUM     0x72
#define BME680_REG_STATUS       0x73
#define BME680_REG_CTRL_MEAS    0x74
#define BME680_REG_CONFIG       0x75
#define BME680_REG_PRESS_MSB    0x1F
#define BME680_REG_GAS_R_MSB    0x2A
#define BME680_CHIP_ID          0x61

BME680Driver::BME680Driver() : address(0), initialized(false) {
}

bool BME680Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    uint8_t chipId = 0;
    if (!readRegister(BME680_REG_CHIP_ID, &chipId) || chipId != BME680_CHIP_ID) {
#if POCKETOS_BME680_ENABLE_LOGGING
        Logger::error("BME680: Invalid chip ID");
#endif
        return false;
    }
    
    writeRegister(BME680_REG_RESET, 0xB6);
    delay(10);
    
    if (!readCalibrationData()) {
#if POCKETOS_BME680_ENABLE_LOGGING
        Logger::error("BME680: Failed to read calibration");
#endif
        return false;
    }
    
    writeRegister(BME680_REG_CTRL_HUM, 0x01);
    writeRegister(BME680_REG_CTRL_MEAS, 0x25);
    writeRegister(BME680_REG_CONFIG, 0x00);
    writeRegister(BME680_REG_CTRL_GAS_1, 0x10);
    
    initialized = true;
#if POCKETOS_BME680_ENABLE_LOGGING
    Logger::info("BME680: Initialized");
#endif
    return true;
}

void BME680Driver::deinit() {
    if (initialized) {
        writeRegister(BME680_REG_CTRL_MEAS, 0x00);
    }
    initialized = false;
}

BME680Data BME680Driver::readData() {
    BME680Data data;
    if (!initialized) return data;
    
    uint8_t buffer[15];
    if (!readRegisters(BME680_REG_PRESS_MSB, buffer, 15)) return data;
    
    uint32_t adc_P = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);
    uint32_t adc_T = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | ((buffer[5] >> 4) & 0x0F);
    uint32_t adc_H = ((uint32_t)buffer[6] << 8) | buffer[7];
    uint32_t adc_G = ((uint32_t)buffer[11] << 2) | ((buffer[12] >> 6) & 0x03);
    
    data.temperature = adc_T / 5120.0f;
    data.pressure = adc_P / 100.0f;
    data.humidity = adc_H / 512.0f;
    data.gas = adc_G / 10.0f;
    data.valid = true;
    
    return data;
}

CapabilitySchema BME680Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x76", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "bme680", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_BME680_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addSignal("gas", ParamType::FLOAT, true, "kOhms");
    schema.addCommand("read", "");
    return schema;
}

String BME680Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "bme680";
    if (name == "tier") return POCKETOS_BME680_TIER_NAME;
    return "";
}

bool BME680Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool BME680Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool BME680Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        *value = Wire.read();
        return true;
    }
    return false;
}

bool BME680Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)len);
    size_t count = 0;
    while (Wire.available() && count < len) {
        buffer[count++] = Wire.read();
    }
    return (count == len);
}

bool BME680Driver::readCalibrationData() {
    return true;
}

#if POCKETOS_BME680_ENABLE_REGISTER_ACCESS
const RegisterDesc* BME680Driver::registers(size_t& count) const {
    static const RegisterDesc BME680_REGISTERS[] = {
        RegisterDesc(0xD0, "CHIP_ID", 1, RegisterAccess::RO, 0x61),
        RegisterDesc(0xE0, "RESET", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0x71, "CTRL_GAS_1", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x72, "CTRL_HUM", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x74, "CTRL_MEAS", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x1F, "PRESS_MSB", 1, RegisterAccess::RO, 0x80),
    };
    count = sizeof(BME680_REGISTERS) / sizeof(RegisterDesc);
    return BME680_REGISTERS;
}

bool BME680Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    return readRegister((uint8_t)reg, buf);
}

bool BME680Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* BME680Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
