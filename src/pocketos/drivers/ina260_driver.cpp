#include "ina260_driver.h"
#include "../driver_config.h"

#if POCKETOS_INA260_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// INA260 Register addresses
#define INA260_REG_CONFIG           0x00
#define INA260_REG_CURRENT          0x01
#define INA260_REG_BUS_VOLTAGE      0x02
#define INA260_REG_POWER            0x03
#define INA260_REG_MASK_ENABLE      0x06
#define INA260_REG_ALERT_LIMIT      0x07
#define INA260_REG_MANUFACTURER_ID  0xFE
#define INA260_REG_DIE_ID           0xFF

// Configuration register bits
#define INA260_CONFIG_RESET         0x8000
#define INA260_CONFIG_DEFAULT       0x6127  // Avg=1, VbusCT=1.1ms, IshCT=1.1ms, continuous

// Device IDs
#define INA260_MANUFACTURER_ID      0x5449  // "TI"
#define INA260_DIE_ID               0x2270

#if POCKETOS_INA260_ENABLE_REGISTER_ACCESS
static const RegisterDesc INA260_REGISTERS[] = {
    RegisterDesc(0x00, "CONFIG", 2, RegisterAccess::RW, INA260_CONFIG_DEFAULT),
    RegisterDesc(0x01, "CURRENT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x02, "BUS_VOLTAGE", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x03, "POWER", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x06, "MASK_ENABLE", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x07, "ALERT_LIMIT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0xFE, "MANUFACTURER_ID", 2, RegisterAccess::RO, INA260_MANUFACTURER_ID),
    RegisterDesc(0xFF, "DIE_ID", 2, RegisterAccess::RO, INA260_DIE_ID)
};
#define INA260_REGISTER_COUNT (sizeof(INA260_REGISTERS) / sizeof(RegisterDesc))
#endif

INA260Driver::INA260Driver() : address(0), initialized(false) {}

bool INA260Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_INA260_ENABLE_LOGGING
    Logger::info("INA260: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Verify manufacturer ID
    uint16_t mfgId;
    if (!readRegister(INA260_REG_MANUFACTURER_ID, &mfgId) || mfgId != INA260_MANUFACTURER_ID) {
#if POCKETOS_INA260_ENABLE_LOGGING
        Logger::error("INA260: Invalid manufacturer ID: 0x" + String(mfgId, HEX));
#endif
        return false;
    }
    
    // Verify die ID
    uint16_t dieId;
    if (!readRegister(INA260_REG_DIE_ID, &dieId) || dieId != INA260_DIE_ID) {
#if POCKETOS_INA260_ENABLE_LOGGING
        Logger::error("INA260: Invalid die ID: 0x" + String(dieId, HEX));
#endif
        return false;
    }
    
    // Reset device
    if (!writeRegister(INA260_REG_CONFIG, INA260_CONFIG_RESET)) {
#if POCKETOS_INA260_ENABLE_LOGGING
        Logger::error("INA260: Failed to reset device");
#endif
        return false;
    }
    delay(1);
    
    // Configure device
    writeRegister(INA260_REG_CONFIG, INA260_CONFIG_DEFAULT);
    
    initialized = true;
#if POCKETOS_INA260_ENABLE_LOGGING
    Logger::info("INA260: Initialized successfully");
#endif
    return true;
}

void INA260Driver::deinit() {
    if (initialized) {
        writeRegister(INA260_REG_CONFIG, 0x0000);  // Power down
    }
    initialized = false;
}

INA260Data INA260Driver::readData() {
    INA260Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint16_t currentRaw, busRaw, powerRaw;
    
    if (!readRegister(INA260_REG_CURRENT, &currentRaw) ||
        !readRegister(INA260_REG_BUS_VOLTAGE, &busRaw) ||
        !readRegister(INA260_REG_POWER, &powerRaw)) {
        return data;
    }
    
    // Convert current (LSB = 1.25mA)
    data.current = (int16_t)currentRaw * 1.25;  // mA
    
    // Convert bus voltage (LSB = 1.25mV)
    data.busVoltage = busRaw * 0.00125;  // V
    
    // Convert power (LSB = 10mW)
    data.power = powerRaw * 10.0;  // mW
    
    data.valid = true;
    return data;
}

#if POCKETOS_INA260_ENABLE_CONFIGURATION
bool INA260Driver::setAveraging(uint8_t samples) {
    if (!initialized) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA260_REG_CONFIG, &config)) {
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
    return writeRegister(INA260_REG_CONFIG, config);
}

bool INA260Driver::setConversionTime(uint16_t microseconds) {
    if (!initialized) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA260_REG_CONFIG, &config)) {
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
    
    // Set both bus voltage and current conversion times
    config &= 0xF800;
    config |= (ctBits << 6) | (ctBits << 3);
    return writeRegister(INA260_REG_CONFIG, config);
}

bool INA260Driver::setContinuousMode(bool continuous) {
    if (!initialized) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA260_REG_CONFIG, &config)) {
        return false;
    }
    
    config &= 0xFFF8;
    config |= continuous ? 0x0007 : 0x0003;  // Continuous vs triggered
    return writeRegister(INA260_REG_CONFIG, config);
}
#endif

#if POCKETOS_INA260_ENABLE_ALERTS
bool INA260Driver::setAlertLimit(float limitValue) {
    if (!initialized) {
        return false;
    }
    
    // Convert limit (using power as example, LSB = 10mW)
    uint16_t limit = (uint16_t)(limitValue / 10.0);
    return writeRegister(INA260_REG_ALERT_LIMIT, limit);
}

bool INA260Driver::enableAlert(bool enable) {
    if (!initialized) {
        return false;
    }
    
    uint16_t mask = enable ? 0x0010 : 0x0000;  // Enable conversion ready flag
    return writeRegister(INA260_REG_MASK_ENABLE, mask);
}

bool INA260Driver::isAlertActive() {
    if (!initialized) {
        return false;
    }
    
    uint16_t mask;
    if (!readRegister(INA260_REG_MASK_ENABLE, &mask)) {
        return false;
    }
    
    return (mask & 0x0008) != 0;  // Check alert function flag
}
#endif

CapabilitySchema INA260Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ina260";
    schema.name = "INA260 Power Monitor";
    schema.tier = POCKETOS_INA260_TIER_NAME;
    
    // Outputs
    schema.outputs.push_back(CapabilityOutput("bus_voltage", "V", "float", "Bus voltage"));
    schema.outputs.push_back(CapabilityOutput("current", "mA", "float", "Current"));
    schema.outputs.push_back(CapabilityOutput("power", "mW", "float", "Power"));
    
#if POCKETOS_INA260_ENABLE_CONFIGURATION
    // Parameters
    schema.parameters.push_back(CapabilityParameter("averaging", "samples", "int", "1", "Number of samples to average"));
    schema.parameters.push_back(CapabilityParameter("continuous", "bool", "bool", "true", "Continuous measurement mode"));
#endif
    
    return schema;
}

String INA260Driver::getParameter(const String& name) {
    if (name == "averaging") return "1";
    if (name == "continuous") return "true";
    return "";
}

bool INA260Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_INA260_ENABLE_CONFIGURATION
    if (name == "averaging") {
        return setAveraging(value.toInt());
    }
    if (name == "continuous") {
        return setContinuousMode(value == "true" || value == "1");
    }
#endif
    return false;
}

#if POCKETOS_INA260_ENABLE_REGISTER_ACCESS
const RegisterDesc* INA260Driver::registers(size_t& count) const {
    count = INA260_REGISTER_COUNT;
    return INA260_REGISTERS;
}

bool INA260Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA260_REGISTERS, INA260_REGISTER_COUNT, reg);
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

bool INA260Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA260_REGISTERS, INA260_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    uint16_t value = ((uint16_t)buf[0] << 8) | buf[1];
    return writeRegister((uint8_t)reg, value);
}

const RegisterDesc* INA260Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(INA260_REGISTERS, INA260_REGISTER_COUNT, name);
}
#endif

bool INA260Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);
    Wire.write(value & 0xFF);
    return Wire.endTransmission() == 0;
}

bool INA260Driver::readRegister(uint8_t reg, uint16_t* value) {
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
