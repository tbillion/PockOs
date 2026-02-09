#include "bme280_driver.h"
#include "../driver_config.h"

#if POCKETOS_BME280_ENABLE_LOGGING
#include "../core/logger.h"
#endif

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
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    lastReadTime = 0;
    readCount = 0;
    errorCount = 0;
#endif
}

bool BME280Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_BME280_ENABLE_LOGGING
    Logger::info("BME280: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check chip ID
    uint8_t chipId = 0;
    if (!readRegister(BME280_REG_CHIP_ID, &chipId)) {
#if POCKETOS_BME280_ENABLE_LOGGING
        Logger::error("BME280: Failed to read chip ID");
#endif
        return false;
    }
    
    if (chipId != BME280_CHIP_ID) {
#if POCKETOS_BME280_ENABLE_LOGGING
        Logger::error("BME280: Invalid chip ID: 0x" + String(chipId, HEX));
#endif
        return false;
    }
    
#if POCKETOS_BME280_ENABLE_CONFIGURATION
    // Soft reset
    writeRegister(BME280_REG_RESET, 0xB6);
    delay(10);
#endif
    
    // Read calibration data
    if (!readCalibrationData()) {
#if POCKETOS_BME280_ENABLE_LOGGING
        Logger::error("BME280: Failed to read calibration data");
#endif
        return false;
    }
    
#if POCKETOS_BME280_ENABLE_CONFIGURATION
    // Configure sensor
    // Humidity oversampling x1
    writeRegister(BME280_REG_CTRL_HUM, 0x01);
    
    // Temperature oversampling x1, Pressure oversampling x1, Normal mode
    writeRegister(BME280_REG_CTRL_MEAS, 0x27);
    
    // Standby time 0.5ms, filter off
    writeRegister(BME280_REG_CONFIG, 0x00);
#else
    // Minimal configuration: just wake up the sensor
    writeRegister(BME280_REG_CTRL_HUM, 0x01);
    writeRegister(BME280_REG_CTRL_MEAS, 0x27);
#endif
    
    initialized = true;
#if POCKETOS_BME280_ENABLE_LOGGING
    Logger::info("BME280: Initialized successfully");
#endif
    return true;
}

void BME280Driver::deinit() {
    // Put sensor in sleep mode
    if (initialized) {
        writeRegister(BME280_REG_CTRL_MEAS, 0x00);
    }
    initialized = false;
#if POCKETOS_BME280_ENABLE_LOGGING
    Logger::info("BME280: Deinitialized");
#endif
}

BME280Data BME280Driver::readData() {
    BME280Data data;
    
    if (!initialized) {
#if POCKETOS_BME280_ENABLE_LOGGING
        Logger::error("BME280: Not initialized");
#endif
        return data;
    }
    
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    uint32_t startTime = millis();
#endif
    
    // Read raw data (8 bytes: press_msb, press_lsb, press_xlsb, temp_msb, temp_lsb, temp_xlsb, hum_msb, hum_lsb)
    uint8_t buffer[8];
    if (!readRegisters(BME280_REG_PRESS_MSB, buffer, 8)) {
#if POCKETOS_BME280_ENABLE_LOGGING
        Logger::error("BME280: Failed to read sensor data");
#endif
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
        errorCount++;
#endif
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
    
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    lastReadTime = millis() - startTime;
    readCount++;
#endif
    
    return data;
}

CapabilitySchema BME280Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x76", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "bme280", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_BME280_TIER_NAME, "", "", "");
    
#if POCKETOS_BME280_ENABLE_OVERSAMPLING_CONFIG
    // Advanced settings (FULL tier only)
    schema.addSetting("oversampling_temp", ParamType::INT, false, "1", "1", "16", "1");
    schema.addSetting("oversampling_press", ParamType::INT, false, "1", "1", "16", "1");
    schema.addSetting("oversampling_hum", ParamType::INT, false, "1", "1", "16", "1");
#endif

#if POCKETOS_BME280_ENABLE_FORCED_MODE
    schema.addSetting("mode", ParamType::ENUM, false, "normal", "", "", "");
#endif

#if POCKETOS_BME280_ENABLE_IIR_FILTER
    schema.addSetting("filter", ParamType::INT, false, "0", "0", "16", "1");
#endif
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    // Diagnostic signals (FULL tier only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
    schema.addSignal("last_read_time", ParamType::INT, true, "ms");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
#if POCKETOS_BME280_ENABLE_CONFIGURATION
    schema.addCommand("reset", "");
#endif
    
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    schema.addCommand("get_diagnostics", "");
#endif
    
    return schema;
}

String BME280Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "bme280";
    } else if (name == "tier") {
        return POCKETOS_BME280_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    } else if (name == "last_read_time") {
        return String(lastReadTime);
    }
#endif
    return "";
}

bool BME280Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_BME280_ENABLE_CONFIGURATION
    // BME280 settings are mostly read-only in this simple implementation
    // Future: Add support for changing oversampling, mode, filter
#if POCKETOS_BME280_ENABLE_LOGGING
    Logger::warn("BME280: Parameter '" + name + "' is read-only");
#endif
#endif
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

#if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
String BME280Driver::getDiagnostics() {
    String diag = "BME280 Diagnostics:\n";
    diag += "  Tier: " + String(POCKETOS_BME280_TIER_NAME) + "\n";
    diag += "  Address: 0x" + String(address, HEX) + "\n";
    diag += "  Initialized: " + String(initialized ? "yes" : "no") + "\n";
    diag += "  Read Count: " + String(readCount) + "\n";
    diag += "  Error Count: " + String(errorCount) + "\n";
    diag += "  Last Read Time: " + String(lastReadTime) + " ms\n";
    diag += "  Success Rate: ";
    if (readCount + errorCount > 0) {
        float successRate = (float)readCount / (readCount + errorCount) * 100.0f;
        diag += String(successRate, 1) + "%\n";
    } else {
        diag += "N/A\n";
    }
    return diag;
}
#endif

#if POCKETOS_BME280_ENABLE_REGISTER_ACCESS
// Complete BME280 register map (Tier 2 only)
static const RegisterDesc BME280_REGISTERS[] = {
    // Calibration registers (Read-only)
    RegisterDesc(0x88, "DIG_T1_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x89, "DIG_T1_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x8A, "DIG_T2_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x8B, "DIG_T2_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x8C, "DIG_T3_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x8D, "DIG_T3_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x8E, "DIG_P1_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x8F, "DIG_P1_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x90, "DIG_P2_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x91, "DIG_P2_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x92, "DIG_P3_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x93, "DIG_P3_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x94, "DIG_P4_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x95, "DIG_P4_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x96, "DIG_P5_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x97, "DIG_P5_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x98, "DIG_P6_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x99, "DIG_P6_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9A, "DIG_P7_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9B, "DIG_P7_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9C, "DIG_P8_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9D, "DIG_P8_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9E, "DIG_P9_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9F, "DIG_P9_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xA1, "DIG_H1", 1, RegisterAccess::RO, 0x00),
    
    // Chip ID (Read-only)
    RegisterDesc(0xD0, "CHIP_ID", 1, RegisterAccess::RO, 0x60),
    
    // Reset register (Write-only)
    RegisterDesc(0xE0, "RESET", 1, RegisterAccess::WO, 0x00),
    
    // Humidity calibration registers (Read-only)
    RegisterDesc(0xE1, "DIG_H2_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE2, "DIG_H2_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE3, "DIG_H3", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE4, "DIG_H4_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE5, "DIG_H4_LSB_H5_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE6, "DIG_H5_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE7, "DIG_H6", 1, RegisterAccess::RO, 0x00),
    
    // Control registers (Read-write)
    RegisterDesc(0xF2, "CTRL_HUM", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xF3, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xF4, "CTRL_MEAS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xF5, "CONFIG", 1, RegisterAccess::RW, 0x00),
    
    // Data registers (Read-only)
    RegisterDesc(0xF7, "PRESS_MSB", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0xF8, "PRESS_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xF9, "PRESS_XLSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xFA, "TEMP_MSB", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0xFB, "TEMP_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xFC, "TEMP_XLSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xFD, "HUM_MSB", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0xFE, "HUM_LSB", 1, RegisterAccess::RO, 0x00),
};

#define BME280_REGISTER_COUNT (sizeof(BME280_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* BME280Driver::registers(size_t& count) const {
    count = BME280_REGISTER_COUNT;
    return BME280_REGISTERS;
}

bool BME280Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    // Find register to validate it exists and is readable
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(BME280_REGISTERS, BME280_REGISTER_COUNT, (uint16_t)reg);
    if (!regDesc) {
        return false; // Register not found
    }
    
    if (!RegisterUtils::isReadable(regDesc->access)) {
        return false; // Register is write-only
    }
    
    // Read the register(s)
    if (len == 1) {
        return readRegister((uint8_t)reg, buf);
    } else {
        return readRegisters((uint8_t)reg, buf, len);
    }
}

bool BME280Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    // Find register to validate it exists and is writable
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(BME280_REGISTERS, BME280_REGISTER_COUNT, (uint16_t)reg);
    if (!regDesc) {
        return false; // Register not found
    }
    
    if (!RegisterUtils::isWritable(regDesc->access)) {
        return false; // Register is read-only
    }
    
    // Write the register
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* BME280Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(BME280_REGISTERS, BME280_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
