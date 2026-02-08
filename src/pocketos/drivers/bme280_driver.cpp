#include "bme280_driver.h"
#include "../core/logger.h"
#include <Wire.h>

namespace PocketOS {

// BME280 Register addresses
#define BME280_REG_DIG_T1         0x88
#define BME280_REG_DIG_H1         0xA1
#define BME280_REG_DIG_H2         0xE1
#define BME280_REG_CHIP_ID        0xD0
#define BME280_REG_RESET          0xE0
#define BME280_REG_CTRL_HUM       0xF2
#define BME280_REG_STATUS         0xF3
#define BME280_REG_CTRL_MEAS      0xF4
#define BME280_REG_CONFIG         0xF5
#define BME280_REG_PRESS_MSB      0xF7

// BME280 Chip ID
#define BME280_CHIP_ID            0x60

BME280Driver::BME280Driver() : address(0), initialized(false) {
    memset(&calibration, 0, sizeof(calibration));
}

bool BME280Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    Logger::info("BME280: Initializing at address 0x" + String(address, HEX));
    
    // Check chip ID
    uint8_t chipId = 0;
    if (!readRegister(BME280_REG_CHIP_ID, &chipId)) {
        Logger::error("BME280: Failed to read chip ID");
        return false;
    }
    
    if (chipId != BME280_CHIP_ID) {
        Logger::error("BME280: Invalid chip ID: 0x" + String(chipId, HEX));
        return false;
    }
    
    // Soft reset
    writeRegister(BME280_REG_RESET, 0xB6);
    delay(10);
    
    // Read calibration data
    if (!readCalibrationData()) {
        Logger::error("BME280: Failed to read calibration data");
        return false;
    }
    
    // Configure sensor
    // Humidity oversampling x1
    writeRegister(BME280_REG_CTRL_HUM, 0x01);
    
    // Temperature oversampling x1, Pressure oversampling x1, Normal mode
    writeRegister(BME280_REG_CTRL_MEAS, 0x27);
    
    // Standby time 0.5ms, filter off
    writeRegister(BME280_REG_CONFIG, 0x00);
    
    initialized = true;
    Logger::info("BME280: Initialized successfully");
    return true;
}

void BME280Driver::deinit() {
    // Put sensor in sleep mode
    if (initialized) {
        writeRegister(BME280_REG_CTRL_MEAS, 0x00);
    }
    initialized = false;
    Logger::info("BME280: Deinitialized");
}

BME280Data BME280Driver::readData() {
    BME280Data data;
    
    if (!initialized) {
        Logger::error("BME280: Not initialized");
        return data;
    }
    
    // Read raw data (8 bytes: press_msb, press_lsb, press_xlsb, temp_msb, temp_lsb, temp_xlsb, hum_msb, hum_lsb)
    uint8_t buffer[8];
    if (!readRegisters(BME280_REG_PRESS_MSB, buffer, 8)) {
        Logger::error("BME280: Failed to read sensor data");
        return data;
    }
    
    // Extract raw values
    int32_t adc_P = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);
    int32_t adc_T = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | ((buffer[5] >> 4) & 0x0F);
    int32_t adc_H = ((uint32_t)buffer[6] << 8) | buffer[7];
    
    // Compensate temperature (must be first)
    int32_t temp = compensateTemperature(adc_T);
    data.temperature = temp / 100.0f;
    
    // Compensate pressure
    uint32_t press = compensatePressure(adc_P);
    data.pressure = press / 25600.0f;  // Convert to hPa
    
    // Compensate humidity
    uint32_t hum = compensateHumidity(adc_H);
    data.humidity = hum / 1024.0f;
    
    data.valid = true;
    return data;
}

CapabilitySchema BME280Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Settings
    schema.addSetting("oversampling_temp", ParamType::INT, false, "1", "1", "16", "1");
    schema.addSetting("oversampling_press", ParamType::INT, false, "1", "1", "16", "1");
    schema.addSetting("oversampling_hum", ParamType::INT, false, "1", "1", "16", "1");
    schema.addSetting("mode", ParamType::ENUM, false, "normal", "", "", "");
    schema.addSetting("filter", ParamType::INT, false, "0", "0", "16", "1");
    
    // Signals (read-only measurements)
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    
    // Commands
    schema.addCommand("read", "");
    schema.addCommand("reset", "");
    
    return schema;
}

String BME280Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "bme280";
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
    return "";
}

bool BME280Driver::setParameter(const String& name, const String& value) {
    // BME280 settings are mostly read-only in this simple implementation
    // Future: Add support for changing oversampling, mode, filter
    Logger::warn("BME280: Parameter '" + name + "' is read-only");
    return false;
}

bool BME280Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool BME280Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool BME280Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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

bool BME280Driver::readCalibrationData() {
    uint8_t buffer[24];
    
    // Read temperature and pressure calibration (0x88-0x9F)
    if (!readRegisters(BME280_REG_DIG_T1, buffer, 24)) {
        return false;
    }
    
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
    
    // Read humidity calibration H1 (0xA1)
    if (!readRegister(BME280_REG_DIG_H1, &calibration.dig_H1)) {
        return false;
    }
    
    // Read humidity calibration H2-H6 (0xE1-0xE7)
    uint8_t hum_calib[7];
    if (!readRegisters(BME280_REG_DIG_H2, hum_calib, 7)) {
        return false;
    }
    
    calibration.dig_H2 = (hum_calib[1] << 8) | hum_calib[0];
    calibration.dig_H3 = hum_calib[2];
    calibration.dig_H4 = (hum_calib[3] << 4) | (hum_calib[4] & 0x0F);
    calibration.dig_H5 = (hum_calib[5] << 4) | (hum_calib[4] >> 4);
    calibration.dig_H6 = hum_calib[6];
    
    return true;
}

int32_t BME280Driver::compensateTemperature(int32_t adc_T) {
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)calibration.dig_T1 << 1))) * ((int32_t)calibration.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calibration.dig_T1)) * ((adc_T >> 4) - ((int32_t)calibration.dig_T1))) >> 12) * ((int32_t)calibration.dig_T3)) >> 14;
    
    calibration.t_fine = var1 + var2;
    T = (calibration.t_fine * 5 + 128) >> 8;
    
    return T;
}

uint32_t BME280Driver::compensatePressure(int32_t adc_P) {
    int64_t var1, var2, p;
    
    var1 = ((int64_t)calibration.t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calibration.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calibration.dig_P5) << 17);
    var2 = var2 + (((int64_t)calibration.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calibration.dig_P3) >> 8) + ((var1 * (int64_t)calibration.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calibration.dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calibration.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calibration.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calibration.dig_P7) << 4);
    
    return (uint32_t)p;
}

uint32_t BME280Driver::compensateHumidity(int32_t adc_H) {
    int32_t v_x1_u32r;
    
    v_x1_u32r = (calibration.t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calibration.dig_H4) << 20) - (((int32_t)calibration.dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)calibration.dig_H6)) >> 10) *
                   (((v_x1_u32r * ((int32_t)calibration.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                   ((int32_t)calibration.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calibration.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    
    return (uint32_t)(v_x1_u32r >> 12);
}

} // namespace PocketOS
