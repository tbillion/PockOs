#include "ina228_driver.h"
#include "../driver_config.h"

#if POCKETOS_INA228_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// INA228 Register addresses
#define INA228_REG_CONFIG           0x00
#define INA228_REG_ADC_CONFIG       0x01
#define INA228_REG_SHUNT_CAL        0x02
#define INA228_REG_SHUNT_TEMPCO     0x03
#define INA228_REG_VSHUNT           0x04
#define INA228_REG_VBUS             0x05
#define INA228_REG_DIETEMP          0x06
#define INA228_REG_CURRENT          0x07
#define INA228_REG_POWER            0x08
#define INA228_REG_ENERGY           0x09
#define INA228_REG_CHARGE           0x0A
#define INA228_REG_DIAG_ALRT        0x0B
#define INA228_REG_SOVL             0x0C
#define INA228_REG_SUVL             0x0D
#define INA228_REG_BOVL             0x0E
#define INA228_REG_BUVL             0x0F
#define INA228_REG_TEMP_LIMIT       0x10
#define INA228_REG_PWR_LIMIT        0x11
#define INA228_REG_MANUFACTURER_ID  0x3E
#define INA228_REG_DEVICE_ID        0x3F

// Device IDs
#define INA228_MANUFACTURER_ID      0x5449  // "TI"
#define INA228_DEVICE_ID            0x2280

#if POCKETOS_INA228_ENABLE_REGISTER_ACCESS
static const RegisterDesc INA228_REGISTERS[] = {
    RegisterDesc(0x00, "CONFIG", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x01, "ADC_CONFIG", 2, RegisterAccess::RW, 0xFB68),
    RegisterDesc(0x02, "SHUNT_CAL", 2, RegisterAccess::RW, 0x1000),
    RegisterDesc(0x03, "SHUNT_TEMPCO", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x04, "VSHUNT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x05, "VBUS", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x06, "DIETEMP", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x07, "CURRENT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x08, "POWER", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x09, "ENERGY", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x0A, "CHARGE", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x0B, "DIAG_ALRT", 2, RegisterAccess::RW, 0x0001),
    RegisterDesc(0x0C, "SOVL", 2, RegisterAccess::RW, 0x7FFF),
    RegisterDesc(0x0D, "SUVL", 2, RegisterAccess::RW, 0x8000),
    RegisterDesc(0x0E, "BOVL", 2, RegisterAccess::RW, 0x7FFF),
    RegisterDesc(0x0F, "BUVL", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x10, "TEMP_LIMIT", 2, RegisterAccess::RW, 0x7FFF),
    RegisterDesc(0x11, "PWR_LIMIT", 2, RegisterAccess::RW, 0xFFFF),
    RegisterDesc(0x3E, "MANUFACTURER_ID", 2, RegisterAccess::RO, INA228_MANUFACTURER_ID),
    RegisterDesc(0x3F, "DEVICE_ID", 2, RegisterAccess::RO, INA228_DEVICE_ID)
};
#define INA228_REGISTER_COUNT (sizeof(INA228_REGISTERS) / sizeof(RegisterDesc))
#endif

INA228Driver::INA228Driver() : address(0), initialized(false), currentLSB(0.001) {}

bool INA228Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_INA228_ENABLE_LOGGING
    Logger::info("INA228: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Verify device ID
    uint16_t devId;
    if (!readRegister(INA228_REG_DEVICE_ID, &devId) || (devId & 0xFFF0) != INA228_DEVICE_ID) {
#if POCKETOS_INA228_ENABLE_LOGGING
        Logger::error("INA228: Invalid device ID: 0x" + String(devId, HEX));
#endif
        return false;
    }
    
    // Reset device
    if (!writeRegister(INA228_REG_CONFIG, 0x8000)) {
#if POCKETOS_INA228_ENABLE_LOGGING
        Logger::error("INA228: Failed to reset device");
#endif
        return false;
    }
    delay(2);
    
#if POCKETOS_INA228_ENABLE_CALIBRATION
    // Default calibration: 0.1Ω shunt, 3.2A max
    setCalibration(0.1, 3.2);
#else
    // Basic configuration
    writeRegister(INA228_REG_ADC_CONFIG, 0xFB68);  // Continuous, 1024 samples avg
    writeRegister(INA228_REG_SHUNT_CAL, 4096);     // Default calibration
#endif
    
    initialized = true;
#if POCKETOS_INA228_ENABLE_LOGGING
    Logger::info("INA228: Initialized successfully");
#endif
    return true;
}

void INA228Driver::deinit() {
    if (initialized) {
        writeRegister(INA228_REG_CONFIG, 0x0000);  // Shutdown
    }
    initialized = false;
}

INA228Data INA228Driver::readData() {
    INA228Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint32_t shuntRaw, busRaw, currentRaw, powerRaw;
    uint16_t tempRaw;
    
    if (!readRegister24(INA228_REG_VSHUNT, &shuntRaw) ||
        !readRegister24(INA228_REG_VBUS, &busRaw) ||
        !readRegister24(INA228_REG_CURRENT, &currentRaw) ||
        !readRegister24(INA228_REG_POWER, &powerRaw) ||
        !readRegister(INA228_REG_DIETEMP, &tempRaw)) {
        return data;
    }
    
    // Convert shunt voltage (20-bit, LSB = 312.5nV)
    int32_t shuntSigned = (shuntRaw & 0x80000) ? (shuntRaw | 0xFFF00000) : shuntRaw;
    data.shuntVoltage = shuntSigned * 0.0003125;  // mV
    
    // Convert bus voltage (20-bit, LSB = 195.3125µV)
    data.busVoltage = (busRaw >> 4) * 0.0001953125;  // V
    
    // Convert current (20-bit)
    int32_t currentSigned = (currentRaw & 0x80000) ? (currentRaw | 0xFFF00000) : currentRaw;
    data.current = currentSigned * currentLSB;  // mA
    
    // Convert power (24-bit)
    data.power = powerRaw * (currentLSB * 3.2);  // mW
    
    // Convert temperature (16-bit, LSB = 7.8125m°C)
    data.temperature = (int16_t)tempRaw * 0.0078125;  // °C
    
    data.valid = true;
    return data;
}

#if POCKETOS_INA228_ENABLE_CALIBRATION
bool INA228Driver::setCalibration(float shuntResistorOhms, float maxCurrentA) {
    if (!initialized) {
        return false;
    }
    
    // Calculate current LSB
    currentLSB = maxCurrentA * 1000.0 / 524288.0;  // 20-bit: mA
    
    // Calculate shunt calibration value
    // SHUNT_CAL = 13107.2 × 10^6 × Current_LSB × Rshunt
    float calValue = 13107.2e6 * (currentLSB / 1000.0) * shuntResistorOhms;
    uint16_t calReg = (uint16_t)(calValue / 4.0);  // Divide by 4 per datasheet
    
#if POCKETOS_INA228_ENABLE_LOGGING
    Logger::info("INA228: Calibration=" + String(calReg) + " CurrentLSB=" + String(currentLSB, 6));
#endif
    
    return writeRegister(INA228_REG_SHUNT_CAL, calReg);
}

bool INA228Driver::setAveraging(uint16_t samples) {
    if (!initialized) {
        return false;
    }
    
    uint16_t adcConfig;
    if (!readRegister(INA228_REG_ADC_CONFIG, &adcConfig)) {
        return false;
    }
    
    // Clear averaging bits and set new value
    adcConfig &= 0xF8FF;
    uint8_t avgBits = 0;
    if (samples >= 1024) avgBits = 7;
    else if (samples >= 512) avgBits = 6;
    else if (samples >= 256) avgBits = 5;
    else if (samples >= 128) avgBits = 4;
    else if (samples >= 64) avgBits = 3;
    else if (samples >= 16) avgBits = 2;
    else if (samples >= 4) avgBits = 1;
    
    adcConfig |= (avgBits << 8);
    return writeRegister(INA228_REG_ADC_CONFIG, adcConfig);
}
#endif

#if POCKETOS_INA228_ENABLE_ALERTS
bool INA228Driver::setAlertLimit(uint8_t alertNum, float limitValue) {
    if (!initialized || alertNum > 5) {
        return false;
    }
    
    uint8_t reg = INA228_REG_SOVL + alertNum;
    uint16_t limit = (uint16_t)(limitValue * 1000);  // Convert to appropriate units
    return writeRegister(reg, limit);
}

bool INA228Driver::enableAlert(uint8_t alertNum, bool enable) {
    if (!initialized || alertNum > 5) {
        return false;
    }
    
    uint16_t diag;
    if (!readRegister(INA228_REG_DIAG_ALRT, &diag)) {
        return false;
    }
    
    if (enable) {
        diag |= (1 << (alertNum + 10));
    } else {
        diag &= ~(1 << (alertNum + 10));
    }
    
    return writeRegister(INA228_REG_DIAG_ALRT, diag);
}
#endif

CapabilitySchema INA228Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ina228";
    schema.name = "INA228 Power Monitor";
    schema.tier = POCKETOS_INA228_TIER_NAME;
    
    // Outputs
    schema.outputs.push_back(CapabilityOutput("bus_voltage", "V", "float", "Bus voltage"));
    schema.outputs.push_back(CapabilityOutput("shunt_voltage", "mV", "float", "Shunt voltage"));
    schema.outputs.push_back(CapabilityOutput("current", "mA", "float", "Current"));
    schema.outputs.push_back(CapabilityOutput("power", "mW", "float", "Power"));
    schema.outputs.push_back(CapabilityOutput("temperature", "°C", "float", "Die temperature"));
    
#if POCKETOS_INA228_ENABLE_CALIBRATION
    // Parameters
    schema.parameters.push_back(CapabilityParameter("shunt_resistor", "ohms", "float", "0.1", "Shunt resistor value"));
    schema.parameters.push_back(CapabilityParameter("max_current", "A", "float", "3.2", "Maximum expected current"));
    schema.parameters.push_back(CapabilityParameter("averaging", "samples", "int", "1024", "Number of samples to average"));
#endif
    
    return schema;
}

String INA228Driver::getParameter(const String& name) {
    if (name == "shunt_resistor") return "0.1";
    if (name == "max_current") return "3.2";
    if (name == "averaging") return "1024";
    return "";
}

bool INA228Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_INA228_ENABLE_CALIBRATION
    if (name == "averaging") {
        return setAveraging(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_INA228_ENABLE_REGISTER_ACCESS
const RegisterDesc* INA228Driver::registers(size_t& count) const {
    count = INA228_REGISTER_COUNT;
    return INA228_REGISTERS;
}

bool INA228Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x3F || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA228_REGISTERS, INA228_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    uint16_t value;
    if (!readRegister((uint8_t)reg, &value)) {
        return false;
    }
    
    buf[0] = (value >> 8) & 0xFF;
    buf[1] = value & 0xFF;
    return true;
}

bool INA228Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x3F || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA228_REGISTERS, INA228_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    uint16_t value = ((uint16_t)buf[0] << 8) | buf[1];
    return writeRegister((uint8_t)reg, value);
}

const RegisterDesc* INA228Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(INA228_REGISTERS, INA228_REGISTER_COUNT, name);
}
#endif

bool INA228Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);
    Wire.write(value & 0xFF);
    return Wire.endTransmission() == 0;
}

bool INA228Driver::readRegister(uint8_t reg, uint16_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)2) != 2) {
        return false;
    }
    
    *value = ((uint16_t)Wire.read() << 8) | Wire.read();
    return true;
}

bool INA228Driver::readRegister24(uint8_t reg, uint32_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)3) != 3) {
        return false;
    }
    
    *value = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    return true;
}

} // namespace PocketOS
