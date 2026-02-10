#include "ina226_driver.h"
#include "../driver_config.h"

#if POCKETOS_INA226_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// INA226 Register addresses
#define INA226_REG_CONFIG           0x00
#define INA226_REG_SHUNT_VOLTAGE    0x01
#define INA226_REG_BUS_VOLTAGE      0x02
#define INA226_REG_POWER            0x03
#define INA226_REG_CURRENT          0x04
#define INA226_REG_CALIBRATION      0x05
#define INA226_REG_MASK_ENABLE      0x06
#define INA226_REG_ALERT_LIMIT      0x07
#define INA226_REG_MANUFACTURER_ID  0xFE
#define INA226_REG_DIE_ID           0xFF

// Configuration register bits
#define INA226_CONFIG_RESET         0x8000
#define INA226_CONFIG_DEFAULT       0x4527  // Avg=1, VbusCT=1.1ms, VshCT=1.1ms, continuous

// Device IDs
#define INA226_MANUFACTURER_ID      0x5449  // "TI"
#define INA226_DIE_ID               0x2260

#if POCKETOS_INA226_ENABLE_REGISTER_ACCESS
static const RegisterDesc INA226_REGISTERS[] = {
    RegisterDesc(0x00, "CONFIG", 2, RegisterAccess::RW, INA226_CONFIG_DEFAULT),
    RegisterDesc(0x01, "SHUNT_VOLTAGE", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x02, "BUS_VOLTAGE", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x03, "POWER", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x04, "CURRENT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x05, "CALIBRATION", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x06, "MASK_ENABLE", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x07, "ALERT_LIMIT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0xFE, "MANUFACTURER_ID", 2, RegisterAccess::RO, INA226_MANUFACTURER_ID),
    RegisterDesc(0xFF, "DIE_ID", 2, RegisterAccess::RO, INA226_DIE_ID)
};
#define INA226_REGISTER_COUNT (sizeof(INA226_REGISTERS) / sizeof(RegisterDesc))
#endif

INA226Driver::INA226Driver() : address(0), initialized(false), currentLSB(0.001), powerLSB(0.025) {}

bool INA226Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_INA226_ENABLE_LOGGING
    Logger::info("INA226: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Verify manufacturer ID
    uint16_t mfgId;
    if (!readRegister(INA226_REG_MANUFACTURER_ID, &mfgId) || mfgId != INA226_MANUFACTURER_ID) {
#if POCKETOS_INA226_ENABLE_LOGGING
        Logger::error("INA226: Invalid manufacturer ID: 0x" + String(mfgId, HEX));
#endif
        return false;
    }
    
    // Reset device
    if (!writeRegister(INA226_REG_CONFIG, INA226_CONFIG_RESET)) {
#if POCKETOS_INA226_ENABLE_LOGGING
        Logger::error("INA226: Failed to reset device");
#endif
        return false;
    }
    delay(1);
    
#if POCKETOS_INA226_ENABLE_CALIBRATION
    // Default calibration: 0.1Ω shunt, 3.2A max
    setCalibration(0.1, 3.2);
#else
    // Basic configuration
    writeRegister(INA226_REG_CONFIG, INA226_CONFIG_DEFAULT);
    writeRegister(INA226_REG_CALIBRATION, 5120);  // Default calibration
#endif
    
    initialized = true;
#if POCKETOS_INA226_ENABLE_LOGGING
    Logger::info("INA226: Initialized successfully");
#endif
    return true;
}

void INA226Driver::deinit() {
    if (initialized) {
        writeRegister(INA226_REG_CONFIG, 0x0000);  // Power down
    }
    initialized = false;
}

INA226Data INA226Driver::readData() {
    INA226Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint16_t shuntRaw, busRaw, powerRaw, currentRaw;
    
    if (!readRegister(INA226_REG_SHUNT_VOLTAGE, &shuntRaw) ||
        !readRegister(INA226_REG_BUS_VOLTAGE, &busRaw) ||
        !readRegister(INA226_REG_POWER, &powerRaw) ||
        !readRegister(INA226_REG_CURRENT, &currentRaw)) {
        return data;
    }
    
    // Convert shunt voltage (LSB = 2.5µV)
    data.shuntVoltage = (int16_t)shuntRaw * 0.0025;  // mV
    
    // Convert bus voltage (LSB = 1.25mV)
    data.busVoltage = busRaw * 0.00125;  // V
    
    // Convert current
    data.current = (int16_t)currentRaw * currentLSB;  // mA
    
    // Convert power
    data.power = powerRaw * powerLSB;  // mW
    
    data.valid = true;
    return data;
}

#if POCKETOS_INA226_ENABLE_CALIBRATION
bool INA226Driver::setCalibration(float shuntResistorOhms, float maxCurrentA) {
    if (!initialized) {
        return false;
    }
    
    // Calculate current LSB: maxCurrent / 32767
    currentLSB = maxCurrentA * 1000.0 / 32767.0;  // mA
    powerLSB = currentLSB * 25.0;  // mW
    
    // Calculate calibration value
    // Cal = 0.00512 / (Current_LSB * Rshunt)
    float calValue = 0.00512 / ((currentLSB / 1000.0) * shuntResistorOhms);
    uint16_t calReg = (uint16_t)calValue;
    
#if POCKETOS_INA226_ENABLE_LOGGING
    Logger::info("INA226: Calibration=" + String(calReg) + " CurrentLSB=" + String(currentLSB, 6));
#endif
    
    return writeRegister(INA226_REG_CALIBRATION, calReg);
}

bool INA226Driver::setAveraging(uint8_t samples) {
    if (!initialized) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA226_REG_CONFIG, &config)) {
        return false;
    }
    
    // Clear averaging bits and set new value
    config &= 0xF1FF;
    uint8_t avgBits = 0;
    if (samples >= 1024) avgBits = 7;
    else if (samples >= 512) avgBits = 6;
    else if (samples >= 256) avgBits = 5;
    else if (samples >= 128) avgBits = 4;
    else if (samples >= 64) avgBits = 3;
    else if (samples >= 16) avgBits = 2;
    else if (samples >= 4) avgBits = 1;
    
    config |= (avgBits << 9);
    return writeRegister(INA226_REG_CONFIG, config);
}

bool INA226Driver::setConversionTime(uint16_t microseconds) {
    if (!initialized) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA226_REG_CONFIG, &config)) {
        return false;
    }
    
    // Determine conversion time bits
    uint8_t ctBits = 4;  // Default 1.1ms
    if (microseconds <= 140) ctBits = 0;
    else if (microseconds <= 204) ctBits = 1;
    else if (microseconds <= 332) ctBits = 2;
    else if (microseconds <= 588) ctBits = 3;
    else if (microseconds <= 1100) ctBits = 4;
    else if (microseconds <= 2116) ctBits = 5;
    else if (microseconds <= 4156) ctBits = 6;
    else ctBits = 7;  // 8.244ms
    
    // Set both bus and shunt conversion times
    config &= 0xF800;
    config |= (ctBits << 6) | (ctBits << 3);
    return writeRegister(INA226_REG_CONFIG, config);
}
#endif

#if POCKETOS_INA226_ENABLE_ALERTS
bool INA226Driver::setAlertLimit(float limitValue) {
    if (!initialized) {
        return false;
    }
    
    // Convert limit based on alert type (assuming power limit)
    uint16_t limit = (uint16_t)(limitValue / powerLSB);
    return writeRegister(INA226_REG_ALERT_LIMIT, limit);
}

bool INA226Driver::enableAlert(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint16_t mask = enable ? 0x0010 : 0x0000;  // Enable conversion ready flag
    return writeRegister(INA226_REG_MASK_ENABLE, mask);
}

bool INA226Driver::isAlertActive() {
    if (!initialized) {
        return false;
    }
    
    uint16_t mask;
    if (!readRegister(INA226_REG_MASK_ENABLE, &mask)) {
        return false;
    }
    
    return (mask & 0x0008) != 0;  // Check alert function flag
}
#endif

CapabilitySchema INA226Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ina226";
    schema.name = "INA226 Power Monitor";
    schema.tier = POCKETOS_INA226_TIER_NAME;
    
    // Outputs
    schema.outputs.push_back(CapabilityOutput("bus_voltage", "V", "float", "Bus voltage"));
    schema.outputs.push_back(CapabilityOutput("shunt_voltage", "mV", "float", "Shunt voltage"));
    schema.outputs.push_back(CapabilityOutput("current", "mA", "float", "Current"));
    schema.outputs.push_back(CapabilityOutput("power", "mW", "float", "Power"));
    
#if POCKETOS_INA226_ENABLE_CALIBRATION
    // Parameters
    schema.parameters.push_back(CapabilityParameter("shunt_resistor", "ohms", "float", "0.1", "Shunt resistor value"));
    schema.parameters.push_back(CapabilityParameter("max_current", "A", "float", "3.2", "Maximum expected current"));
    schema.parameters.push_back(CapabilityParameter("averaging", "samples", "int", "1", "Number of samples to average"));
#endif
    
    return schema;
}

String INA226Driver::getParameter(const String& name) {
    if (name == "shunt_resistor") return "0.1";
    if (name == "max_current") return "3.2";
    if (name == "averaging") return "1";
    return "";
}

bool INA226Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_INA226_ENABLE_CALIBRATION
    if (name == "averaging") {
        return setAveraging(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_INA226_ENABLE_REGISTER_ACCESS
const RegisterDesc* INA226Driver::registers(size_t& count) const {
    count = INA226_REGISTER_COUNT;
    return INA226_REGISTERS;
}

bool INA226Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA226_REGISTERS, INA226_REGISTER_COUNT, reg);
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

bool INA226Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA226_REGISTERS, INA226_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    uint16_t value = ((uint16_t)buf[0] << 8) | buf[1];
    return writeRegister((uint8_t)reg, value);
}

const RegisterDesc* INA226Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(INA226_REGISTERS, INA226_REGISTER_COUNT, name);
}
#endif

bool INA226Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);
    Wire.write(value & 0xFF);
    return Wire.endTransmission() == 0;
}

bool INA226Driver::readRegister(uint8_t reg, uint16_t* value) {
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

} // namespace PocketOS
