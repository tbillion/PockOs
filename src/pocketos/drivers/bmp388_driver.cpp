#include "bmp388_driver.h"
#include "../driver_config.h"
#if POCKETOS_BMP388_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define BMP388_REG_CHIP_ID 0x00
#define BMP388_REG_DATA    0x04
#define BMP388_REG_PWR_CTRL 0x1B
#define BMP388_CHIP_ID     0x50

BMP388Driver::BMP388Driver() : address(0), initialized(false) {
    memset(&calibration, 0, sizeof(calibration));
}

bool BMP388Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    uint8_t chipId = 0;
    if (!readRegister(BMP388_REG_CHIP_ID, &chipId) || chipId != BMP388_CHIP_ID) return false;
    if (!readCalibrationData()) return false;
    writeRegister(BMP388_REG_PWR_CTRL, 0x33);
    delay(10);
    initialized = true;
    return true;
}

void BMP388Driver::deinit() {
    writeRegister(BMP388_REG_PWR_CTRL, 0x00);
    initialized = false;
}

BMP388Data BMP388Driver::readData() {
    BMP388Data data;
    if (!initialized) return data;
    
    uint8_t buffer[6];
    if (!readRegisters(BMP388_REG_DATA, buffer, 6)) return data;
    
    uint32_t adc_P = ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[1] << 8) | buffer[0];
    uint32_t adc_T = ((uint32_t)buffer[5] << 16) | ((uint32_t)buffer[4] << 8) | buffer[3];
    
    data.temperature = compensateTemperature(adc_T);
    data.pressure = compensatePressure(adc_P);
    data.valid = true;
    
    return data;
}

CapabilitySchema BMP388Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x76", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "bmp388", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_BMP388_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addCommand("read", "");
    return schema;
}

String BMP388Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "bmp388";
    if (name == "tier") return POCKETOS_BMP388_TIER_NAME;
    return "";
}

bool BMP388Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool BMP388Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool BMP388Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool BMP388Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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

bool BMP388Driver::readCalibrationData() {
    uint8_t buffer[21];
    if (!readRegisters(0x31, buffer, 21)) return false;
    
    calibration.par_t1 = (buffer[1] << 8) | buffer[0];
    calibration.par_t2 = (buffer[3] << 8) | buffer[2];
    calibration.par_t3 = buffer[4];
    calibration.par_p1 = (buffer[6] << 8) | buffer[5];
    calibration.par_p2 = (buffer[8] << 8) | buffer[7];
    calibration.par_p3 = buffer[9];
    calibration.par_p4 = buffer[10];
    calibration.par_p5 = (buffer[12] << 8) | buffer[11];
    calibration.par_p6 = (buffer[14] << 8) | buffer[13];
    calibration.par_p7 = buffer[15];
    calibration.par_p8 = buffer[16];
    calibration.par_p9 = (buffer[18] << 8) | buffer[17];
    calibration.par_p10 = buffer[19];
    calibration.par_p11 = buffer[20];
    
    return true;
}

float BMP388Driver::compensateTemperature(uint32_t adc_T) {
    float partial_data1 = (float)(adc_T - calibration.par_t1);
    float partial_data2 = (float)(partial_data1 * calibration.par_t2);
    calibration.t_lin = (int64_t)(partial_data2 + (partial_data1 * partial_data1) * calibration.par_t3);
    return (float)calibration.t_lin / 65536.0f;
}

float BMP388Driver::compensatePressure(uint32_t adc_P) {
    float partial_data1 = (float)calibration.t_lin * (float)calibration.t_lin;
    float partial_data2 = partial_data1 / 64.0f;
    float partial_data3 = (partial_data2 / 256.0f) * (float)calibration.par_p8;
    float partial_data4 = (float)calibration.par_p7 * partial_data1 * 16.0f;
    float offset = (float)calibration.par_p6 * 4194304.0f + partial_data3 + partial_data4;
    
    partial_data2 = ((float)calibration.par_p4 / 8192.0f) * partial_data1;
    partial_data3 = ((float)calibration.par_p3 / 256.0f) * partial_data1 * partial_data1;
    partial_data4 = (float)calibration.par_p2 * partial_data1 * 4.0f;
    float sensitivity = ((float)calibration.par_p1 - 16384.0f) * 70368744177664.0f + partial_data2 + partial_data3 + partial_data4;
    
    float comp_press = (offset / sensitivity) * ((float)adc_P - 8388608.0f);
    
    return comp_press / 100.0f;
}

#if POCKETOS_BMP388_ENABLE_REGISTER_ACCESS
const RegisterDesc* BMP388Driver::registers(size_t& count) const {
    static const RegisterDesc BMP388_REGISTERS[] = {
        RegisterDesc(0x00, "CHIP_ID", 1, RegisterAccess::RO, 0x50),
        RegisterDesc(0x04, "DATA_0", 1, RegisterAccess::RO, 0x00),
        RegisterDesc(0x1B, "PWR_CTRL", 1, RegisterAccess::RW, 0x00),
    };
    count = sizeof(BMP388_REGISTERS) / sizeof(RegisterDesc);
    return BMP388_REGISTERS;
}

bool BMP388Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    return readRegister((uint8_t)reg, buf);
}

bool BMP388Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* BMP388Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
