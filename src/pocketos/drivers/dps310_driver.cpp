#include "dps310_driver.h"
#include "../driver_config.h"
#if POCKETOS_DPS310_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define DPS310_REG_PSR_B2      0x00
#define DPS310_REG_TMP_B2      0x03
#define DPS310_REG_PRS_CFG     0x06
#define DPS310_REG_TMP_CFG     0x07
#define DPS310_REG_MEAS_CFG    0x08
#define DPS310_REG_CFG_REG     0x09
#define DPS310_REG_RESET       0x0C
#define DPS310_REG_ID          0x0D
#define DPS310_REG_COEF        0x10
#define DPS310_CHIP_ID         0x10

DPS310Driver::DPS310Driver() : address(0), initialized(false) {
    memset(&calibration, 0, sizeof(calibration));
}

bool DPS310Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    uint8_t chipId = 0;
    if (!readRegister(DPS310_REG_ID, &chipId) || (chipId & 0xF0) != DPS310_CHIP_ID) {
#if POCKETOS_DPS310_ENABLE_LOGGING
        Logger::error("DPS310: Invalid chip ID");
#endif
        return false;
    }
    
    if (!readCalibrationData()) {
#if POCKETOS_DPS310_ENABLE_LOGGING
        Logger::error("DPS310: Failed to read calibration");
#endif
        return false;
    }
    
    writeRegister(DPS310_REG_PRS_CFG, 0x00);
    writeRegister(DPS310_REG_TMP_CFG, 0x80);
    writeRegister(DPS310_REG_MEAS_CFG, 0x07);
    writeRegister(DPS310_REG_CFG_REG, 0x00);
    
    initialized = true;
#if POCKETOS_DPS310_ENABLE_LOGGING
    Logger::info("DPS310: Initialized");
#endif
    return true;
}

void DPS310Driver::deinit() {
    if (initialized) {
        writeRegister(DPS310_REG_MEAS_CFG, 0x00);
    }
    initialized = false;
}

DPS310Data DPS310Driver::readData() {
    DPS310Data data;
    if (!initialized) return data;
    
    uint8_t buffer[6];
    if (!readRegisters(DPS310_REG_PSR_B2, buffer, 6)) return data;
    
    int32_t raw_psr = ((int32_t)buffer[0] << 16) | ((int32_t)buffer[1] << 8) | buffer[2];
    if (raw_psr & 0x800000) raw_psr |= 0xFF000000;
    
    int32_t raw_tmp = ((int32_t)buffer[3] << 16) | ((int32_t)buffer[4] << 8) | buffer[5];
    if (raw_tmp & 0x800000) raw_tmp |= 0xFF000000;
    
    float temp_scaled = compensateTemperature(raw_tmp);
    data.temperature = temp_scaled;
    data.pressure = compensatePressure(raw_psr, temp_scaled);
    data.valid = true;
    
    return data;
}

CapabilitySchema DPS310Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x77", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "dps310", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_DPS310_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addCommand("read", "");
    return schema;
}

String DPS310Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "dps310";
    if (name == "tier") return POCKETOS_DPS310_TIER_NAME;
    return "";
}

bool DPS310Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool DPS310Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool DPS310Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool DPS310Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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

bool DPS310Driver::readCalibrationData() {
    uint8_t coef[18];
    if (!readRegisters(DPS310_REG_COEF, coef, 18)) return false;
    
    calibration.c0 = ((int32_t)coef[0] << 4) | (((int32_t)coef[1] >> 4) & 0x0F);
    if (calibration.c0 & 0x800) calibration.c0 |= 0xFFFFF000;
    
    calibration.c1 = (((int32_t)coef[1] & 0x0F) << 8) | (int32_t)coef[2];
    if (calibration.c1 & 0x800) calibration.c1 |= 0xFFFFF000;
    
    calibration.c00 = ((int32_t)coef[3] << 12) | ((int32_t)coef[4] << 4) | (((int32_t)coef[5] >> 4) & 0x0F);
    if (calibration.c00 & 0x80000) calibration.c00 |= 0xFFF00000;
    
    calibration.c10 = (((int32_t)coef[5] & 0x0F) << 16) | ((int32_t)coef[6] << 8) | (int32_t)coef[7];
    if (calibration.c10 & 0x80000) calibration.c10 |= 0xFFF00000;
    
    calibration.c01 = ((int32_t)coef[8] << 8) | (int32_t)coef[9];
    if (calibration.c01 & 0x8000) calibration.c01 |= 0xFFFF0000;
    
    calibration.c11 = ((int32_t)coef[10] << 8) | (int32_t)coef[11];
    if (calibration.c11 & 0x8000) calibration.c11 |= 0xFFFF0000;
    
    calibration.c20 = ((int32_t)coef[12] << 8) | (int32_t)coef[13];
    if (calibration.c20 & 0x8000) calibration.c20 |= 0xFFFF0000;
    
    calibration.c21 = ((int32_t)coef[14] << 8) | (int32_t)coef[15];
    if (calibration.c21 & 0x8000) calibration.c21 |= 0xFFFF0000;
    
    calibration.c30 = ((int32_t)coef[16] << 8) | (int32_t)coef[17];
    if (calibration.c30 & 0x8000) calibration.c30 |= 0xFFFF0000;
    
    return true;
}

float DPS310Driver::compensateTemperature(int32_t raw) {
    float temp_scaled = (float)raw / 524288.0f;
    return (float)calibration.c0 * 0.5f + (float)calibration.c1 * temp_scaled;
}

float DPS310Driver::compensatePressure(int32_t raw, float temp_scaled) {
    float prs_scaled = (float)raw / 524288.0f;
    float prs = (float)calibration.c00 + prs_scaled * ((float)calibration.c10 + prs_scaled * ((float)calibration.c20 + prs_scaled * (float)calibration.c30));
    prs += temp_scaled * ((float)calibration.c01 + prs_scaled * ((float)calibration.c11 + prs_scaled * (float)calibration.c21));
    return prs / 100.0f;
}

#if POCKETOS_DPS310_ENABLE_REGISTER_ACCESS
const RegisterDesc* DPS310Driver::registers(size_t& count) const {
    static const RegisterDesc DPS310_REGISTERS[] = {
        RegisterDesc(0x00, "PSR_B2", 1, RegisterAccess::RO, 0x00),
        RegisterDesc(0x03, "TMP_B2", 1, RegisterAccess::RO, 0x00),
        RegisterDesc(0x06, "PRS_CFG", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x07, "TMP_CFG", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x08, "MEAS_CFG", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x0D, "PROD_ID", 1, RegisterAccess::RO, 0x10),
    };
    count = sizeof(DPS310_REGISTERS) / sizeof(RegisterDesc);
    return DPS310_REGISTERS;
}

bool DPS310Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    return readRegister((uint8_t)reg, buf);
}

bool DPS310Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* DPS310Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
