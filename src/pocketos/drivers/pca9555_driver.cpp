#include "pca9555_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_PCA9555_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_PCA9555_ENABLE_REGISTER_ACCESS
static const RegisterDesc PCA9555_REGISTERS[] = {
    RegisterDesc(0x00, "INPUT0", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x01, "INPUT1", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x02, "OUTPUT0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x03, "OUTPUT1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x04, "POLARITY0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "POLARITY1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "CONFIG0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x07, "CONFIG1", 1, RegisterAccess::RW, 0xFF),
};
#define PCA9555_REGISTER_COUNT (sizeof(PCA9555_REGISTERS) / sizeof(RegisterDesc))
#endif

PCA9555Driver::PCA9555Driver() 
    : address(0), initialized(false)
#if POCKETOS_PCA9555_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool PCA9555Driver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Set all pins as inputs by default
    if (!writeRegister(PCA9555_REG_CONFIG0, 0xFF)) {
        return false;
    }
    if (!writeRegister(PCA9555_REG_CONFIG1, 0xFF)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void PCA9555Driver::deinit() {
    initialized = false;
}

uint8_t PCA9555Driver::getPortReg(uint8_t pin, uint8_t reg0, uint8_t reg1) {
    return (pin < 8) ? reg0 : reg1;
}

bool PCA9555Driver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCA9555_REG_CONFIG0, PCA9555_REG_CONFIG1);
    uint8_t bit = pin % 8;
    
    uint8_t config;
    if (!readRegister(reg, &config)) {
        return false;
    }
    
    if (mode == INPUT || mode == INPUT_PULLUP) {
        config |= (1 << bit);
    } else {
        config &= ~(1 << bit);
    }
    
    return writeRegister(reg, config);
}

bool PCA9555Driver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCA9555_REG_OUTPUT0, PCA9555_REG_OUTPUT1);
    uint8_t bit = pin % 8;
    
    uint8_t output;
    if (!readRegister(reg, &output)) {
        return false;
    }
    
    if (value) {
        output |= (1 << bit);
    } else {
        output &= ~(1 << bit);
    }
    
    return writeRegister(reg, output);
}

int PCA9555Driver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 15) {
        return -1;
    }
    
    uint8_t reg = getPortReg(pin, PCA9555_REG_INPUT0, PCA9555_REG_INPUT1);
    uint8_t bit = pin % 8;
    
    uint8_t input;
    if (!readRegister(reg, &input)) {
        return -1;
    }
    
    return (input & (1 << bit)) ? HIGH : LOW;
}

bool PCA9555Driver::writePort(uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    if (!writeRegister(PCA9555_REG_OUTPUT0, value & 0xFF)) {
        return false;
    }
    
    return writeRegister(PCA9555_REG_OUTPUT1, (value >> 8) & 0xFF);
}

uint16_t PCA9555Driver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t port0, port1;
    if (!readRegister(PCA9555_REG_INPUT0, &port0)) {
        return 0;
    }
    if (!readRegister(PCA9555_REG_INPUT1, &port1)) {
        return 0;
    }
    
    return ((uint16_t)port1 << 8) | port0;
}

bool PCA9555Driver::writePort0(uint8_t value) {
    return initialized && writeRegister(PCA9555_REG_OUTPUT0, value);
}

bool PCA9555Driver::writePort1(uint8_t value) {
    return initialized && writeRegister(PCA9555_REG_OUTPUT1, value);
}

uint8_t PCA9555Driver::readPort0() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t value;
    if (!readRegister(PCA9555_REG_INPUT0, &value)) {
        return 0;
    }
    return value;
}

uint8_t PCA9555Driver::readPort1() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t value;
    if (!readRegister(PCA9555_REG_INPUT1, &value)) {
        return 0;
    }
    return value;
}

#if POCKETOS_PCA9555_ENABLE_CONFIGURATION
bool PCA9555Driver::setPolarity(uint8_t pin, bool inverted) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCA9555_REG_POLARITY0, PCA9555_REG_POLARITY1);
    uint8_t bit = pin % 8;
    
    uint8_t polarity;
    if (!readRegister(reg, &polarity)) {
        return false;
    }
    
    if (inverted) {
        polarity |= (1 << bit);
    } else {
        polarity &= ~(1 << bit);
    }
    
    return writeRegister(reg, polarity);
}
#endif

CapabilitySchema PCA9555Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "pca9555";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_PCA9555_TIER_NAME;
    schema.description = "PCA9555 16-bit I/O expander";
    
    schema.capabilities.push_back(Capability("gpio_pins", "16", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    
#if POCKETOS_PCA9555_ENABLE_CONFIGURATION
    schema.capabilities.push_back(Capability("polarity", "true", "Input polarity inversion"));
#endif
    
    return schema;
}

String PCA9555Driver::getParameter(const String& name) {
    return "";
}

bool PCA9555Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_PCA9555_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCA9555Driver::registers(size_t& count) const {
    count = PCA9555_REGISTER_COUNT;
    return PCA9555_REGISTERS;
}

bool PCA9555Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x07 || len != 1) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool PCA9555Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x07 || len != 1) {
        return false;
    }
    
    // INPUT registers are read-only
    if (reg == PCA9555_REG_INPUT0 || reg == PCA9555_REG_INPUT1) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* PCA9555Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCA9555_REGISTERS, PCA9555_REGISTER_COUNT, name);
}
#endif

// Private methods

bool PCA9555Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_PCA9555_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool PCA9555Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
#if POCKETOS_PCA9555_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
#if POCKETOS_PCA9555_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    *value = Wire.read();
    
#if POCKETOS_PCA9555_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
