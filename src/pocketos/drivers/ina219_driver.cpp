#include "ina219_driver.h"
#include "../driver_config.h"

#if POCKETOS_INA219_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// INA219 Register addresses
#define INA219_REG_CONFIG           0x00
#define INA219_REG_SHUNT_VOLTAGE    0x01
#define INA219_REG_BUS_VOLTAGE      0x02
#define INA219_REG_POWER            0x03
#define INA219_REG_CURRENT          0x04
#define INA219_REG_CALIBRATION      0x05

// Configuration register bits
#define INA219_CONFIG_RESET         0x8000
#define INA219_CONFIG_DEFAULT       0x399F  // 32V, ±320mV, 12-bit, continuous

#if POCKETOS_INA219_ENABLE_REGISTER_ACCESS
static const RegisterDesc INA219_REGISTERS[] = {
    RegisterDesc(0x00, "CONFIG", 2, RegisterAccess::RW, INA219_CONFIG_DEFAULT),
    RegisterDesc(0x01, "SHUNT_VOLTAGE", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x02, "BUS_VOLTAGE", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x03, "POWER", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x04, "CURRENT", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x05, "CALIBRATION", 2, RegisterAccess::RW, 0x0000)
};
#define INA219_REGISTER_COUNT (sizeof(INA219_REGISTERS) / sizeof(RegisterDesc))
#endif

INA219Driver::INA219Driver() : address(0), initialized(false), currentLSB(0.001), powerLSB(0.02) {}

bool INA219Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_INA219_ENABLE_LOGGING
    Logger::info("INA219: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Reset device
    if (!writeRegister(INA219_REG_CONFIG, INA219_CONFIG_RESET)) {
#if POCKETOS_INA219_ENABLE_LOGGING
        Logger::error("INA219: Failed to reset device");
#endif
        return false;
    }
    delay(1);
    
#if POCKETOS_INA219_ENABLE_CALIBRATION
    // Default calibration: 0.1Ω shunt, 3.2A max
    setCalibration(0.1, 3.2);
#else
    // Basic configuration
    writeRegister(INA219_REG_CONFIG, INA219_CONFIG_DEFAULT);
    writeRegister(INA219_REG_CALIBRATION, 4096);  // Default calibration
#endif
    
    initialized = true;
#if POCKETOS_INA219_ENABLE_LOGGING
    Logger::info("INA219: Initialized successfully");
#endif
    return true;
}

void INA219Driver::deinit() {
    if (initialized) {
        writeRegister(INA219_REG_CONFIG, 0x0000);  // Power down
    }
    initialized = false;
}

INA219Data INA219Driver::readData() {
    INA219Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint16_t shuntRaw, busRaw, powerRaw, currentRaw;
    
    if (!readRegister(INA219_REG_SHUNT_VOLTAGE, &shuntRaw) ||
        !readRegister(INA219_REG_BUS_VOLTAGE, &busRaw) ||
        !readRegister(INA219_REG_POWER, &powerRaw) ||
        !readRegister(INA219_REG_CURRENT, &currentRaw)) {
        return data;
    }
    
    // Convert shunt voltage (LSB = 10µV)
    data.shuntVoltage = (int16_t)shuntRaw * 0.01;  // mV
    
    // Convert bus voltage (LSB = 4mV)
    data.busVoltage = (busRaw >> 3) * 0.004;  // V
    
    // Convert current
    data.current = (int16_t)currentRaw * currentLSB;  // mA
    
    // Convert power
    data.power = powerRaw * powerLSB;  // mW
    
    data.valid = true;
    return data;
}

#if POCKETOS_INA219_ENABLE_CALIBRATION
bool INA219Driver::setCalibration(float shuntResistorOhms, float maxCurrentA) {
    if (!initialized) {
        return false;
    }
    
    // Calculate current LSB: maxCurrent / 32767
    currentLSB = maxCurrentA * 1000.0 / 32767.0;  // mA
    powerLSB = currentLSB * 20.0;  // mW
    
    // Calculate calibration value
    // Cal = 0.04096 / (Current_LSB * Rshunt)
    float calValue = 0.04096 / ((currentLSB / 1000.0) * shuntResistorOhms);
    uint16_t calReg = (uint16_t)calValue;
    
#if POCKETOS_INA219_ENABLE_LOGGING
    Logger::info("INA219: Calibration=" + String(calReg) + " CurrentLSB=" + String(currentLSB, 6));
#endif
    
    return writeRegister(INA219_REG_CALIBRATION, calReg);
}
#endif

CapabilitySchema INA219Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ina219";
    schema.name = "INA219 Power Monitor";
    schema.tier = POCKETOS_INA219_TIER_NAME;
    
    // Outputs
    schema.outputs.push_back(CapabilityOutput("bus_voltage", "V", "float", "Bus voltage"));
    schema.outputs.push_back(CapabilityOutput("shunt_voltage", "mV", "float", "Shunt voltage"));
    schema.outputs.push_back(CapabilityOutput("current", "mA", "float", "Current"));
    schema.outputs.push_back(CapabilityOutput("power", "mW", "float", "Power"));
    
#if POCKETOS_INA219_ENABLE_CALIBRATION
    // Parameters
    schema.parameters.push_back(CapabilityParameter("shunt_resistor", "ohms", "float", "0.1", "Shunt resistor value"));
    schema.parameters.push_back(CapabilityParameter("max_current", "A", "float", "3.2", "Maximum expected current"));
#endif
    
    return schema;
}

String INA219Driver::getParameter(const String& name) {
    if (name == "shunt_resistor") return "0.1";
    if (name == "max_current") return "3.2";
    return "";
}

bool INA219Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_INA219_ENABLE_CALIBRATION
    if (name == "shunt_resistor" || name == "max_current") {
        // Would need to store these and recalibrate
        return false;  // Not implemented in basic version
    }
#endif
    return false;
}

#if POCKETOS_INA219_ENABLE_REGISTER_ACCESS
const RegisterDesc* INA219Driver::registers(size_t& count) const {
    count = INA219_REGISTER_COUNT;
    return INA219_REGISTERS;
}

bool INA219Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA219_REGISTERS, INA219_REGISTER_COUNT, reg);
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

bool INA219Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 2) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(INA219_REGISTERS, INA219_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    uint16_t value = ((uint16_t)buf[0] << 8) | buf[1];
    return writeRegister((uint8_t)reg, value);
}

const RegisterDesc* INA219Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(INA219_REGISTERS, INA219_REGISTER_COUNT, name);
}
#endif

bool INA219Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);
    Wire.write(value & 0xFF);
    return Wire.endTransmission() == 0;
}

bool INA219Driver::readRegister(uint8_t reg, uint16_t* value) {
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
