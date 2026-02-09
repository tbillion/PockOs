#include "ina3221_driver.h"
#include "../driver_config.h"

#if POCKETOS_INA3221_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// INA3221 Register addresses
#define INA3221_REG_CONFIG          0x00
#define INA3221_REG_CH1_SHUNT       0x01
#define INA3221_REG_CH1_BUS         0x02
#define INA3221_REG_CH2_SHUNT       0x03
#define INA3221_REG_CH2_BUS         0x04
#define INA3221_REG_CH3_SHUNT       0x05
#define INA3221_REG_CH3_BUS         0x06
#define INA3221_REG_CH1_CRIT_ALERT  0x07
#define INA3221_REG_CH1_WARN_ALERT  0x08
#define INA3221_REG_CH2_CRIT_ALERT  0x09
#define INA3221_REG_CH2_WARN_ALERT  0x0A
#define INA3221_REG_CH3_CRIT_ALERT  0x0B
#define INA3221_REG_CH3_WARN_ALERT  0x0C
#define INA3221_REG_SHUNT_SUM       0x0D
#define INA3221_REG_SHUNT_SUM_LIMIT 0x0E
#define INA3221_REG_MASK_ENABLE     0x0F
#define INA3221_REG_POWER_LIMIT_UB  0x10
#define INA3221_REG_POWER_LIMIT_LB  0x11
#define INA3221_REG_MANUFACTURER_ID 0xFE
#define INA3221_REG_DIE_ID          0xFF

// Configuration register bits
#define INA3221_CONFIG_RESET        0x8000
#define INA3221_CONFIG_DEFAULT      0x7127  // All channels enabled, continuous

// Device IDs
#define INA3221_MANUFACTURER_ID     0x5449  // "TI"
#define INA3221_DIE_ID              0x3220

#if POCKETOS_INA3221_ENABLE_REGISTER_ACCESS
static const RegisterDesc INA3221_REGISTERS[] = {
    RegisterDesc(0x00, "CONFIG", 2, RegisterAccess::RW, INA3221_CONFIG_DEFAULT),
    RegisterDesc(0x01, "CH1_SHUNT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x02, "CH1_BUS", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x03, "CH2_SHUNT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x04, "CH2_BUS", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x05, "CH3_SHUNT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x06, "CH3_BUS", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x07, "CH1_CRIT_ALERT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x08, "CH1_WARN_ALERT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x09, "CH2_CRIT_ALERT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x0A, "CH2_WARN_ALERT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x0B, "CH3_CRIT_ALERT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x0C, "CH3_WARN_ALERT", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x0D, "SHUNT_SUM", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x0E, "SHUNT_SUM_LIMIT", 2, RegisterAccess::RW, 0x7FFF),
    RegisterDesc(0x0F, "MASK_ENABLE", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x10, "POWER_LIMIT_UB", 2, RegisterAccess::RW, 0x7FFF),
    RegisterDesc(0x11, "POWER_LIMIT_LB", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0xFE, "MANUFACTURER_ID", 2, RegisterAccess::RO, INA3221_MANUFACTURER_ID),
    RegisterDesc(0xFF, "DIE_ID", 2, RegisterAccess::RO, INA3221_DIE_ID)
};
#define INA3221_REGISTER_COUNT (sizeof(INA3221_REGISTERS) / sizeof(RegisterDesc))
#endif

INA3221Driver::INA3221Driver() : address(0), initialized(false) {
    // Default 0.1Ω shunt resistors
    shuntResistor[0] = 0.1;
    shuntResistor[1] = 0.1;
    shuntResistor[2] = 0.1;
}

bool INA3221Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_INA3221_ENABLE_LOGGING
    Logger::info("INA3221: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Verify manufacturer ID
    uint16_t mfgId;
    if (!readRegister(INA3221_REG_MANUFACTURER_ID, &mfgId) || mfgId != INA3221_MANUFACTURER_ID) {
#if POCKETOS_INA3221_ENABLE_LOGGING
        Logger::error("INA3221: Invalid manufacturer ID: 0x" + String(mfgId, HEX));
#endif
        return false;
    }
    
    // Verify die ID
    uint16_t dieId;
    if (!readRegister(INA3221_REG_DIE_ID, &dieId) || dieId != INA3221_DIE_ID) {
#if POCKETOS_INA3221_ENABLE_LOGGING
        Logger::error("INA3221: Invalid die ID: 0x" + String(dieId, HEX));
#endif
        return false;
    }
    
    // Reset device
    if (!writeRegister(INA3221_REG_CONFIG, INA3221_CONFIG_RESET)) {
#if POCKETOS_INA3221_ENABLE_LOGGING
        Logger::error("INA3221: Failed to reset device");
#endif
        return false;
    }
    delay(1);
    
    // Configure device
    writeRegister(INA3221_REG_CONFIG, INA3221_CONFIG_DEFAULT);
    
    initialized = true;
#if POCKETOS_INA3221_ENABLE_LOGGING
    Logger::info("INA3221: Initialized successfully");
#endif
    return true;
}

void INA3221Driver::deinit() {
    if (initialized) {
        writeRegister(INA3221_REG_CONFIG, 0x0000);  // Power down
    }
    initialized = false;
}

INA3221Data INA3221Driver::readData() {
    INA3221Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read all three channels
    for (uint8_t i = 0; i < 3; i++) {
        data.channel[i] = readChannel(i);
    }
    
    data.valid = data.channel[0].valid || data.channel[1].valid || data.channel[2].valid;
    return data;
}

INA3221ChannelData INA3221Driver::readChannel(uint8_t channel) {
    INA3221ChannelData data;
    
    if (!initialized || channel > 2) {
        return data;
    }
    
    uint8_t shuntReg = INA3221_REG_CH1_SHUNT + (channel * 2);
    uint8_t busReg = INA3221_REG_CH1_BUS + (channel * 2);
    
    uint16_t shuntRaw, busRaw;
    
    if (!readRegister(shuntReg, &shuntRaw) || !readRegister(busReg, &busRaw)) {
        return data;
    }
    
    // Convert shunt voltage (LSB = 40µV)
    int16_t shuntSigned = (int16_t)(shuntRaw >> 3);  // 13-bit signed
    data.shuntVoltage = shuntSigned * 0.04;  // mV
    
    // Convert bus voltage (LSB = 8mV)
    data.busVoltage = (busRaw >> 3) * 0.008;  // V
    
    // Calculate current using shunt resistor
    data.current = data.shuntVoltage / shuntResistor[channel];  // mA
    
    data.valid = true;
    return data;
}

#if POCKETOS_INA3221_ENABLE_CONFIGURATION
bool INA3221Driver::setShuntResistor(uint8_t channel, float resistorOhms) {
    if (channel > 2) {
        return false;
    }
    
    shuntResistor[channel] = resistorOhms;
    
#if POCKETOS_INA3221_ENABLE_LOGGING
    Logger::info("INA3221: Channel " + String(channel + 1) + " shunt=" + String(resistorOhms, 3) + "Ω");
#endif
    
    return true;
}

bool INA3221Driver::setAveraging(uint8_t samples) {
    if (!initialized) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA3221_REG_CONFIG, &config)) {
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
    return writeRegister(INA3221_REG_CONFIG, config);
}

bool INA3221Driver::setConversionTime(uint16_t microseconds) {
    if (!initialized) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA3221_REG_CONFIG, &config)) {
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
    config &= 0xF803;
    config |= (ctBits << 6) | (ctBits << 3);
    return writeRegister(INA3221_REG_CONFIG, config);
}

bool INA3221Driver::enableChannel(uint8_t channel, bool enable) {
    if (!initialized || channel > 2) {
        return false;
    }
    
    uint16_t config;
    if (!readRegister(INA3221_REG_CONFIG, &config)) {
        return false;
    }
    
    uint16_t channelBit = (1 << (14 - channel));
    if (enable) {
        config |= channelBit;
    } else {
        config &= ~channelBit;
    }
    
    return writeRegister(INA3221_REG_CONFIG, config);
}
#endif

#if POCKETOS_INA3221_ENABLE_ALERTS
bool INA3221Driver::setWarningLimit(uint8_t channel, float limitV) {
    if (!initialized || channel > 2) {
        return false;
    }
    
    uint8_t reg = INA3221_REG_CH1_WARN_ALERT + (channel * 2);
    // Convert to register format (LSB = 8mV)
    int16_t limit = (int16_t)(limitV / 0.008) << 3;
    return writeRegister(reg, (uint16_t)limit);
}

bool INA3221Driver::setCriticalLimit(uint8_t channel, float limitV) {
    if (!initialized || channel > 2) {
        return false;
    }
    
    uint8_t reg = INA3221_REG_CH1_CRIT_ALERT + (channel * 2);
    // Convert to register format (LSB = 8mV)
    int16_t limit = (int16_t)(limitV / 0.008) << 3;
    return writeRegister(reg, (uint16_t)limit);
}

bool INA3221Driver::enableWarningAlert(uint8_t channel, bool enable) {
    if (!initialized || channel > 2) {
        return false;
    }
    
    uint16_t mask;
    if (!readRegister(INA3221_REG_MASK_ENABLE, &mask)) {
        return false;
    }
    
    uint16_t bit = (1 << (11 - channel));
    if (enable) {
        mask |= bit;
    } else {
        mask &= ~bit;
    }
    
    return writeRegister(INA3221_REG_MASK_ENABLE, mask);
}

bool INA3221Driver::enableCriticalAlert(uint8_t channel, bool enable) {
    if (!initialized || channel > 2) {
        return false;
    }
    
    uint16_t mask;
    if (!readRegister(INA3221_REG_MASK_ENABLE, &mask)) {
        return false;
    }
    
    uint16_t bit = (1 << (14 - channel));
    if (enable) {
        mask |= bit;
    } else {
        mask &= ~bit;
    }
    
    return writeRegister(INA3221_REG_MASK_ENABLE, mask);
}
#endif

CapabilitySchema INA3221Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ina3221";
    schema.name = "INA3221 3-Channel Power Monitor";
    schema.tier = POCKETOS_INA3221_TIER_NAME;
    
    // Outputs for all 3 channels
    for (int ch = 1; ch <= 3; ch++) {
        String prefix = "ch" + String(ch) + "_";
        schema.outputs.push_back(CapabilityOutput(prefix + "bus_voltage", "V", "float", "Channel " + String(ch) + " bus voltage"));
        schema.outputs.push_back(CapabilityOutput(prefix + "shunt_voltage", "mV", "float", "Channel " + String(ch) + " shunt voltage"));
        schema.outputs.push_back(CapabilityOutput(prefix + "current", "mA", "float", "Channel " + String(ch) + " current"));
    }
    
#if POCKETOS_INA3221_ENABLE_CONFIGURATION
    // Parameters
    for (int ch = 1; ch <= 3; ch++) {
        String prefix = "ch" + String(ch) + "_";
        schema.parameters.push_back(CapabilityParameter(prefix + "shunt_resistor", "ohms", "float", "0.1", 
            "Channel " + String(ch) + " shunt resistor value"));
    }
    schema.parameters.push_back(CapabilityParameter("averaging", "samples", "int", "1", "Number of samples to average"));
#endif
    
    return schema;
}

String INA3221Driver::getParameter(const String& name) {
    if (name.startsWith("ch") && name.endsWith("_shunt_resistor")) {
        int ch = name.substring(2, 3).toInt();
        if (ch >= 1 && ch <= 3) {
            return String(shuntResistor[ch - 1], 3);
        }
    }
    if (name == "averaging") return "1";
    return "";
}

bool INA3221Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_INA3221_ENABLE_CONFIGURATION
    if (name.startsWith("ch") && name.endsWith("_shunt_resistor")) {
        int ch = name.substring(2, 3).toInt();
        if (ch >= 1 && ch <= 3) {
            return setShuntResistor(ch - 1, value.toFloat());
        }
    }
    if (name == "averaging") {
        return setAveraging(value.toInt());
    }
#endif
    return false;
}

#if POCKETOS_INA3221_ENABLE_REGISTER_ACCESS
const RegisterDesc* INA3221Driver::registers(size_t& count) const {
    count = INA3221_REGISTER_COUNT;
    return INA3221_REGISTERS;
}

bool INA3221Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA3221_REGISTERS, INA3221_REGISTER_COUNT, reg);
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

bool INA3221Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA3221_REGISTERS, INA3221_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    uint16_t value = ((uint16_t)buf[0] << 8) | buf[1];
    return writeRegister((uint8_t)reg, value);
}

const RegisterDesc* INA3221Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(INA3221_REGISTERS, INA3221_REGISTER_COUNT, name);
}
#endif

bool INA3221Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);
    Wire.write(value & 0xFF);
    return Wire.endTransmission() == 0;
}

bool INA3221Driver::readRegister(uint8_t reg, uint16_t* value) {
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
