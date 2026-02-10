#include "bmp280_driver.h"
#include "../driver_config.h"
#if POCKETOS_BMP280_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define BMP280_REG_DIG_T1  0x88
#define BMP280_REG_CHIP_ID 0xD0
#define BMP280_REG_RESET   0xE0
#define BMP280_REG_CTRL    0xF4
#define BMP280_REG_DATA    0xF7
#define BMP280_CHIP_ID     0x58

BMP280Driver::BMP280Driver() : address(0), initialized(false) {
    memset(&calibration, 0, sizeof(calibration));
}

bool BMP280Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    uint8_t chipId = 0;
    if (!readRegister(BMP280_REG_CHIP_ID, &chipId) || chipId != BMP280_CHIP_ID) return false;
    if (!readCalibrationData()) return false;
    writeRegister(BMP280_REG_CTRL, 0x27);
    initialized = true;
    return true;
}

void BMP280Driver::deinit() {
    initialized = false;
}

BMP280Data BMP280Driver::readData() {
    BMP280Data data;
    if (!initialized) return data;
    
    uint8_t buffer[6];
    if (!readRegisters(BMP280_REG_DATA, buffer, 6)) return data;
    
    int32_t adc_P = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);
    int32_t adc_T = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | ((buffer[5] >> 4) & 0x0F);
    
    data.temperature = compensateTemperature(adc_T) / 100.0f;
    data.pressure = compensatePressure(adc_P) / 25600.0f;
    data.valid = true;
    
    return data;
}

CapabilitySchema BMP280Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x76", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "bmp280", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_BMP280_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addCommand("read", "");
    return schema;
}

String BMP280Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "bmp280";
    if (name == "tier") return POCKETOS_BMP280_TIER_NAME;
    return "";
}

bool BMP280Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool BMP280Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool BMP280Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool BMP280Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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

bool BMP280Driver::readCalibrationData() {
    uint8_t buffer[24];
    if (!readRegisters(BMP280_REG_DIG_T1, buffer, 24)) return false;
    
    calibration.dig_T1 = (buffer[1] << 8) | buffer[0];
    calibration.dig_T2 = (buffer[3] << 8) | buffer[2];
    calibration.dig_T3 = (buffer[5] << 8) | buffer[4];
    calibration.dig_P1 = (buffer[7] << 8) | buffer[6];
    calibration.dig_P2 = (buffer[9] << 8) | buffer[8];
    calibration.dig_P3 = (buffer[11] << 8) | buffer[10];
    calibration.dig_P4 = (buffer[13] << 8) | buffer[12];
    calibration.dig_P5 = (buffer[15] << 8) | buffer[14];
    calibration.dig_P6 = (buffer[17] << 8) | buffer[16];
    calibration.dig_P7 = (buffer[19] << 8) | buffer[18];
    calibration.dig_P8 = (buffer[21] << 8) | buffer[20];
    calibration.dig_P9 = (buffer[23] << 8) | buffer[22];
    
    return true;
}

int32_t BMP280Driver::compensateTemperature(int32_t adc_T) {
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)calibration.dig_T1 << 1))) * ((int32_t)calibration.dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)calibration.dig_T1)) * ((adc_T >> 4) - ((int32_t)calibration.dig_T1))) >> 12) * ((int32_t)calibration.dig_T3)) >> 14;
    calibration.t_fine = var1 + var2;
    return (calibration.t_fine * 5 + 128) >> 8;
}

uint32_t BMP280Driver::compensatePressure(int32_t adc_P) {
    int64_t var1 = ((int64_t)calibration.t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)calibration.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calibration.dig_P5) << 17);
    var2 = var2 + (((int64_t)calibration.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calibration.dig_P3) >> 8) + ((var1 * (int64_t)calibration.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calibration.dig_P1) >> 33;
    
    if (var1 == 0) return 0;
    
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calibration.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calibration.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calibration.dig_P7) << 4);
    
    return (uint32_t)p;
}

#if POCKETOS_BMP280_ENABLE_REGISTER_ACCESS
const RegisterDesc* BMP280Driver::registers(size_t& count) const {
    static const RegisterDesc BMP280_REGISTERS[] = {
        RegisterDesc(0xD0, "CHIP_ID", 1, RegisterAccess::RO, 0x58),
        RegisterDesc(0xE0, "RESET", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0xF4, "CTRL_MEAS", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0xF7, "PRESS_MSB", 1, RegisterAccess::RO, 0x80),
    };
    count = sizeof(BMP280_REGISTERS) / sizeof(RegisterDesc);
    return BMP280_REGISTERS;
}

bool BMP280Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    return readRegister((uint8_t)reg, buf);
}

bool BMP280Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* BMP280Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
