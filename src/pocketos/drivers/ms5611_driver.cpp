#include "ms5611_driver.h"
#include "../driver_config.h"
#if POCKETOS_MS5611_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define MS5611_CMD_RESET       0x1E
#define MS5611_CMD_CONV_D1     0x48
#define MS5611_CMD_CONV_D2     0x58
#define MS5611_CMD_ADC_READ    0x00
#define MS5611_CMD_PROM_READ   0xA0

MS5611Driver::MS5611Driver() : address(0), initialized(false) {
    memset(&calibration, 0, sizeof(calibration));
}

bool MS5611Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    if (!sendCommand(MS5611_CMD_RESET)) {
#if POCKETOS_MS5611_ENABLE_LOGGING
        Logger::error("MS5611: Reset failed");
#endif
        return false;
    }
    delay(10);
    
    if (!readCalibrationData()) {
#if POCKETOS_MS5611_ENABLE_LOGGING
        Logger::error("MS5611: Failed to read calibration");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_MS5611_ENABLE_LOGGING
    Logger::info("MS5611: Initialized");
#endif
    return true;
}

void MS5611Driver::deinit() {
    initialized = false;
}

MS5611Data MS5611Driver::readData() {
    MS5611Data data;
    if (!initialized) return data;
    
    uint32_t D2 = readRawTemperature();
    uint32_t D1 = readRawPressure();
    
    if (D1 == 0 || D2 == 0) return data;
    
    int32_t dT = D2 - ((int32_t)calibration.c5 << 8);
    int32_t TEMP = 2000 + (((int64_t)dT * calibration.c6) >> 23);
    
    int64_t OFF = ((int64_t)calibration.c2 << 16) + (((int64_t)calibration.c4 * dT) >> 7);
    int64_t SENS = ((int64_t)calibration.c1 << 15) + (((int64_t)calibration.c3 * dT) >> 8);
    
    if (TEMP < 2000) {
        int32_t T2 = ((int64_t)dT * dT) >> 31;
        int64_t OFF2 = (5 * (TEMP - 2000) * (TEMP - 2000)) >> 1;
        int64_t SENS2 = (5 * (TEMP - 2000) * (TEMP - 2000)) >> 2;
        
        if (TEMP < -1500) {
            OFF2 += 7 * (TEMP + 1500) * (TEMP + 1500);
            SENS2 += (11 * (TEMP + 1500) * (TEMP + 1500)) >> 1;
        }
        
        TEMP -= T2;
        OFF -= OFF2;
        SENS -= SENS2;
    }
    
    int32_t P = (((D1 * SENS) >> 21) - OFF) >> 15;
    
    data.temperature = TEMP / 100.0f;
    data.pressure = P / 100.0f;
    data.valid = true;
    
    return data;
}

CapabilitySchema MS5611Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x77", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "ms5611", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_MS5611_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addCommand("read", "");
    return schema;
}

String MS5611Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "ms5611";
    if (name == "tier") return POCKETOS_MS5611_TIER_NAME;
    return "";
}

bool MS5611Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool MS5611Driver::sendCommand(uint8_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(cmd);
    return (Wire.endTransmission() == 0);
}

bool MS5611Driver::readADC(uint32_t* value) {
    Wire.beginTransmission(address);
    Wire.write(MS5611_CMD_ADC_READ);
    if (Wire.endTransmission() != 0) return false;
    
    Wire.requestFrom(address, (uint8_t)3);
    if (Wire.available() >= 3) {
        *value = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
        return true;
    }
    return false;
}

bool MS5611Driver::readCalibrationData() {
    for (uint8_t i = 0; i < 6; i++) {
        Wire.beginTransmission(address);
        Wire.write(MS5611_CMD_PROM_READ + (i + 1) * 2);
        if (Wire.endTransmission() != 0) return false;
        
        Wire.requestFrom(address, (uint8_t)2);
        if (Wire.available() >= 2) {
            uint16_t value = ((uint16_t)Wire.read() << 8) | Wire.read();
            switch (i) {
                case 0: calibration.c1 = value; break;
                case 1: calibration.c2 = value; break;
                case 2: calibration.c3 = value; break;
                case 3: calibration.c4 = value; break;
                case 4: calibration.c5 = value; break;
                case 5: calibration.c6 = value; break;
            }
        } else {
            return false;
        }
    }
    return true;
}

uint32_t MS5611Driver::readRawTemperature() {
    if (!sendCommand(MS5611_CMD_CONV_D2)) return 0;
    delay(10);
    uint32_t value = 0;
    readADC(&value);
    return value;
}

uint32_t MS5611Driver::readRawPressure() {
    if (!sendCommand(MS5611_CMD_CONV_D1)) return 0;
    delay(10);
    uint32_t value = 0;
    readADC(&value);
    return value;
}

#if POCKETOS_MS5611_ENABLE_REGISTER_ACCESS
const RegisterDesc* MS5611Driver::registers(size_t& count) const {
    static const RegisterDesc MS5611_REGISTERS[] = {
        RegisterDesc(0x1E, "RESET", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0x48, "CONV_D1", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0x58, "CONV_D2", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0xA0, "PROM_C1", 2, RegisterAccess::RO, 0x00),
    };
    count = sizeof(MS5611_REGISTERS) / sizeof(RegisterDesc);
    return MS5611_REGISTERS;
}

bool MS5611Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return false;
}

bool MS5611Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return sendCommand((uint8_t)reg);
}

const RegisterDesc* MS5611Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
