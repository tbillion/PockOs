#include "bmp085_driver.h"
#include "../driver_config.h"

#if POCKETOS_BMP085_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define BMP085_REG_CAL_AC1    0xAA
#define BMP085_REG_CONTROL    0xF4
#define BMP085_REG_RESULT     0xF6
#define BMP085_REG_CHIP_ID    0xD0

#define BMP085_CHIP_ID        0x55
#define BMP085_CMD_TEMP       0x2E
#define BMP085_CMD_PRESS      0x34

BMP085Driver::BMP085Driver() : address(0), initialized(false), oversampling(0) {
    memset(&calibration, 0, sizeof(calibration));
}

bool BMP085Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_BMP085_ENABLE_LOGGING
    Logger::info("BMP085: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t chipId = 0;
    if (!readRegister(BMP085_REG_CHIP_ID, &chipId)) {
        return false;
    }
    
    if (chipId != BMP085_CHIP_ID) {
        return false;
    }
    
    if (!readCalibrationData()) {
        return false;
    }
    
    initialized = true;
    return true;
}

void BMP085Driver::deinit() {
    initialized = false;
}

BMP085Data BMP085Driver::readData() {
    BMP085Data data;
    
    if (!initialized) {
        return data;
    }
    
    int32_t rawTemp = readRawTemperature();
    int32_t rawPress = readRawPressure();
    
    // Temperature calculation
    int32_t x1 = ((rawTemp - calibration.ac6) * calibration.ac5) >> 15;
    int32_t x2 = (calibration.mc << 11) / (x1 + calibration.md);
    int32_t b5 = x1 + x2;
    data.temperature = ((b5 + 8) >> 4) / 10.0;
    
    // Pressure calculation
    int32_t b6 = b5 - 4000;
    x1 = (calibration.b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = (calibration.ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((calibration.ac1 * 4 + x3) << oversampling) + 2) / 4;
    
    x1 = (calibration.ac3 * b6) >> 13;
    x2 = (calibration.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = (calibration.ac4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)rawPress - b3) * (50000 >> oversampling);
    
    int32_t p;
    if (b7 < 0x80000000) {
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }
    
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);
    
    data.pressure = p / 100.0;
    data.valid = true;
    
    return data;
}

CapabilitySchema BMP085Driver::getSchema() const {
    CapabilitySchema schema;
    
    schema.addSetting("address", ParamType::STRING, true, "0x77", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "bmp085", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_BMP085_TIER_NAME, "", "", "");
    
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    
    schema.addCommand("read", "");
    
    return schema;
}

String BMP085Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "bmp085";
    } else if (name == "tier") {
        return POCKETOS_BMP085_TIER_NAME;
    }
    return "";
}

bool BMP085Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool BMP085Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool BMP085Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        *value = Wire.read();
        return true;
    }
    
    return false;
}

bool BMP085Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)len);
    size_t count = 0;
    while (Wire.available() && count < len) {
        buffer[count++] = Wire.read();
    }
    
    return (count == len);
}

bool BMP085Driver::readCalibrationData() {
    uint8_t buffer[22];
    if (!readRegisters(BMP085_REG_CAL_AC1, buffer, 22)) {
        return false;
    }
    
    calibration.ac1 = (buffer[0] << 8) | buffer[1];
    calibration.ac2 = (buffer[2] << 8) | buffer[3];
    calibration.ac3 = (buffer[4] << 8) | buffer[5];
    calibration.ac4 = (buffer[6] << 8) | buffer[7];
    calibration.ac5 = (buffer[8] << 8) | buffer[9];
    calibration.ac6 = (buffer[10] << 8) | buffer[11];
    calibration.b1 = (buffer[12] << 8) | buffer[13];
    calibration.b2 = (buffer[14] << 8) | buffer[15];
    calibration.mb = (buffer[16] << 8) | buffer[17];
    calibration.mc = (buffer[18] << 8) | buffer[19];
    calibration.md = (buffer[20] << 8) | buffer[21];
    
    return true;
}

int32_t BMP085Driver::readRawTemperature() {
    writeRegister(BMP085_REG_CONTROL, BMP085_CMD_TEMP);
    delay(5);
    
    uint8_t buffer[2];
    readRegisters(BMP085_REG_RESULT, buffer, 2);
    return (buffer[0] << 8) | buffer[1];
}

int32_t BMP085Driver::readRawPressure() {
    writeRegister(BMP085_REG_CONTROL, BMP085_CMD_PRESS + (oversampling << 6));
    delay(2 + (3 << oversampling));
    
    uint8_t buffer[3];
    readRegisters(BMP085_REG_RESULT, buffer, 3);
    return ((buffer[0] << 16) | (buffer[1] << 8) | buffer[2]) >> (8 - oversampling);
}

#if POCKETOS_BMP085_ENABLE_REGISTER_ACCESS
const RegisterDesc* BMP085Driver::registers(size_t& count) const {
    static const RegisterDesc BMP085_REGISTERS[] = {
        RegisterDesc(0xD0, "CHIP_ID", 1, RegisterAccess::RO, 0x55),
        RegisterDesc(0xF4, "CTRL_MEAS", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0xF6, "OUT_MSB", 1, RegisterAccess::RO, 0x00),
    };
    count = sizeof(BMP085_REGISTERS) / sizeof(RegisterDesc);
    return BMP085_REGISTERS;
}

bool BMP085Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    return readRegister((uint8_t)reg, buf);
}

bool BMP085Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* BMP085Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
