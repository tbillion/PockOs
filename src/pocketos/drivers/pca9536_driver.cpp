#include "pca9536_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_PCA9536_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_PCA9536_ENABLE_REGISTER_ACCESS
static const RegisterDesc PCA9536_REGISTERS[] = {
    RegisterDesc(0x00, "INPUT", 1, RegisterAccess::RO, 0x0F),
    RegisterDesc(0x01, "OUTPUT", 1, RegisterAccess::RW, 0x0F),
    RegisterDesc(0x02, "POLARITY", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "CONFIG", 1, RegisterAccess::RW, 0x0F),
};
#define PCA9536_REGISTER_COUNT (sizeof(PCA9536_REGISTERS) / sizeof(RegisterDesc))
#endif

PCA9536Driver::PCA9536Driver() 
    : address(0), initialized(false)
#if POCKETOS_PCA9536_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool PCA9536Driver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Set all pins as inputs by default
    if (!writeRegister(PCA9536_REG_CONFIG, 0x0F)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void PCA9536Driver::deinit() {
    initialized = false;
}

bool PCA9536Driver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 3) {
        return false;
    }
    
    uint8_t config;
    if (!readRegister(PCA9536_REG_CONFIG, &config)) {
        return false;
    }
    
    if (mode == INPUT || mode == INPUT_PULLUP) {
        config |= (1 << pin);
    } else {
        config &= ~(1 << pin);
    }
    
    return writeRegister(PCA9536_REG_CONFIG, config);
}

bool PCA9536Driver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 3) {
        return false;
    }
    
    uint8_t output;
    if (!readRegister(PCA9536_REG_OUTPUT, &output)) {
        return false;
    }
    
    if (value) {
        output |= (1 << pin);
    } else {
        output &= ~(1 << pin);
    }
    
    return writeRegister(PCA9536_REG_OUTPUT, output);
}

int PCA9536Driver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 3) {
        return -1;
    }
    
    uint8_t input;
    if (!readRegister(PCA9536_REG_INPUT, &input)) {
        return -1;
    }
    
    return (input & (1 << pin)) ? HIGH : LOW;
}

bool PCA9536Driver::writePort(uint8_t value) {
    if (!initialized) {
        return false;
    }
    
    return writeRegister(PCA9536_REG_OUTPUT, value & 0x0F);
}

uint8_t PCA9536Driver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t input;
    if (!readRegister(PCA9536_REG_INPUT, &input)) {
        return 0;
    }
    
    return input & 0x0F;
}

#if POCKETOS_PCA9536_ENABLE_CONFIGURATION
bool PCA9536Driver::setPolarity(uint8_t pin, bool inverted) {
    if (!initialized || pin > 3) {
        return false;
    }
    
    uint8_t polarity;
    if (!readRegister(PCA9536_REG_POLARITY, &polarity)) {
        return false;
    }
    
    if (inverted) {
        polarity |= (1 << pin);
    } else {
        polarity &= ~(1 << pin);
    }
    
    return writeRegister(PCA9536_REG_POLARITY, polarity);
}
#endif

CapabilitySchema PCA9536Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "pca9536";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_PCA9536_TIER_NAME;
    schema.description = "PCA9536 4-bit I/O expander";
    
    schema.capabilities.push_back(Capability("gpio_pins", "4", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    
#if POCKETOS_PCA9536_ENABLE_CONFIGURATION
    schema.capabilities.push_back(Capability("polarity", "true", "Input polarity inversion"));
#endif
    
    return schema;
}

String PCA9536Driver::getParameter(const String& name) {
    return "";
}

bool PCA9536Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_PCA9536_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCA9536Driver::registers(size_t& count) const {
    count = PCA9536_REGISTER_COUNT;
    return PCA9536_REGISTERS;
}

bool PCA9536Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x03 || len != 1) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool PCA9536Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x03 || len != 1) {
        return false;
    }
    
    // INPUT register is read-only
    if (reg == PCA9536_REG_INPUT) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* PCA9536Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCA9536_REGISTERS, PCA9536_REGISTER_COUNT, name);
}
#endif

// Private methods

bool PCA9536Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_PCA9536_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool PCA9536Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
#if POCKETOS_PCA9536_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
#if POCKETOS_PCA9536_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    *value = Wire.read();
    
#if POCKETOS_PCA9536_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
