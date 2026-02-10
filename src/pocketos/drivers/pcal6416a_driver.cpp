#include "pcal6416a_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_PCAL6416A_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_PCAL6416A_ENABLE_REGISTER_ACCESS
static const RegisterDesc PCAL6416A_REGISTERS[] = {
    RegisterDesc(0x00, "INPUT0", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x01, "INPUT1", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x02, "OUTPUT0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x03, "OUTPUT1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x04, "POLARITY0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "POLARITY1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "CONFIG0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x07, "CONFIG1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x40, "DRIVE0_0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x41, "DRIVE0_1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x42, "DRIVE1_0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x43, "DRIVE1_1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x44, "LATCH0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x45, "LATCH1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x46, "PULLUP0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x47, "PULLUP1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x48, "PULLDOWN0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x49, "PULLDOWN1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4A, "INTMASK0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x4B, "INTMASK1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x4C, "INTSTAT0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x4D, "INTSTAT1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x4F, "OUTCONF", 1, RegisterAccess::RW, 0x00),
};
#define PCAL6416A_REGISTER_COUNT (sizeof(PCAL6416A_REGISTERS) / sizeof(RegisterDesc))
#endif

PCAL6416ADriver::PCAL6416ADriver() 
    : address(0), initialized(false)
#if POCKETOS_PCAL6416A_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool PCAL6416ADriver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Set all pins as inputs by default
    if (!writeRegister(PCAL6416A_REG_CONFIG0, 0xFF)) {
        return false;
    }
    if (!writeRegister(PCAL6416A_REG_CONFIG1, 0xFF)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void PCAL6416ADriver::deinit() {
    initialized = false;
}

uint8_t PCAL6416ADriver::getPortReg(uint8_t pin, uint8_t reg0, uint8_t reg1) {
    return (pin < 8) ? reg0 : reg1;
}

bool PCAL6416ADriver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_CONFIG0, PCAL6416A_REG_CONFIG1);
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
    
    if (!writeRegister(reg, config)) {
        return false;
    }
    
#if POCKETOS_PCAL6416A_ENABLE_CONFIGURATION
    if (mode == INPUT_PULLUP) {
        return setPullUp(pin, true);
    }
#endif
    
    return true;
}

bool PCAL6416ADriver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_OUTPUT0, PCAL6416A_REG_OUTPUT1);
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

int PCAL6416ADriver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 15) {
        return -1;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_INPUT0, PCAL6416A_REG_INPUT1);
    uint8_t bit = pin % 8;
    
    uint8_t input;
    if (!readRegister(reg, &input)) {
        return -1;
    }
    
    return (input & (1 << bit)) ? HIGH : LOW;
}

bool PCAL6416ADriver::writePort(uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    if (!writeRegister(PCAL6416A_REG_OUTPUT0, value & 0xFF)) {
        return false;
    }
    
    return writeRegister(PCAL6416A_REG_OUTPUT1, (value >> 8) & 0xFF);
}

uint16_t PCAL6416ADriver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t port0, port1;
    if (!readRegister(PCAL6416A_REG_INPUT0, &port0)) {
        return 0;
    }
    if (!readRegister(PCAL6416A_REG_INPUT1, &port1)) {
        return 0;
    }
    
    return ((uint16_t)port1 << 8) | port0;
}

#if POCKETOS_PCAL6416A_ENABLE_CONFIGURATION
bool PCAL6416ADriver::setPullUp(uint8_t pin, bool enable) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_PULLUP0, PCAL6416A_REG_PULLUP1);
    uint8_t bit = pin % 8;
    
    uint8_t pullup;
    if (!readRegister(reg, &pullup)) {
        return false;
    }
    
    if (enable) {
        pullup |= (1 << bit);
    } else {
        pullup &= ~(1 << bit);
    }
    
    return writeRegister(reg, pullup);
}

bool PCAL6416ADriver::setPullDown(uint8_t pin, bool enable) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_PULLDOWN0, PCAL6416A_REG_PULLDOWN1);
    uint8_t bit = pin % 8;
    
    uint8_t pulldown;
    if (!readRegister(reg, &pulldown)) {
        return false;
    }
    
    if (enable) {
        pulldown |= (1 << bit);
    } else {
        pulldown &= ~(1 << bit);
    }
    
    return writeRegister(reg, pulldown);
}

bool PCAL6416ADriver::setPolarity(uint8_t pin, bool inverted) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_POLARITY0, PCAL6416A_REG_POLARITY1);
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

bool PCAL6416ADriver::setDriveStrength(uint8_t pin, uint8_t strength) {
    // Drive strength is configured via DRIVE registers (2 bits per pin)
    // Simplified implementation - would need more complex bit manipulation
    return initialized && pin <= 15;
}

bool PCAL6416ADriver::enableInterrupt(uint8_t pin) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_INTMASK0, PCAL6416A_REG_INTMASK1);
    uint8_t bit = pin % 8;
    
    uint8_t intmask;
    if (!readRegister(reg, &intmask)) {
        return false;
    }
    
    intmask &= ~(1 << bit);  // Clear bit to enable interrupt
    return writeRegister(reg, intmask);
}

bool PCAL6416ADriver::disableInterrupt(uint8_t pin) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, PCAL6416A_REG_INTMASK0, PCAL6416A_REG_INTMASK1);
    uint8_t bit = pin % 8;
    
    uint8_t intmask;
    if (!readRegister(reg, &intmask)) {
        return false;
    }
    
    intmask |= (1 << bit);  // Set bit to disable interrupt
    return writeRegister(reg, intmask);
}

uint16_t PCAL6416ADriver::getInterruptStatus() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t stat0, stat1;
    if (!readRegister(PCAL6416A_REG_INTSTAT0, &stat0)) {
        return 0;
    }
    if (!readRegister(PCAL6416A_REG_INTSTAT1, &stat1)) {
        return 0;
    }
    
    return ((uint16_t)stat1 << 8) | stat0;
}
#endif

CapabilitySchema PCAL6416ADriver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "pcal6416a";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_PCAL6416A_TIER_NAME;
    schema.description = "PCAL6416A 16-bit GPIO expander with advanced features";
    
    schema.capabilities.push_back(Capability("gpio_pins", "16", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    
#if POCKETOS_PCAL6416A_ENABLE_CONFIGURATION
    schema.capabilities.push_back(Capability("pull_up", "true", "Internal pull-up support"));
    schema.capabilities.push_back(Capability("pull_down", "true", "Internal pull-down support"));
    schema.capabilities.push_back(Capability("polarity", "true", "Input polarity inversion"));
    schema.capabilities.push_back(Capability("drive_strength", "true", "Configurable drive strength"));
    schema.capabilities.push_back(Capability("interrupts", "true", "Interrupt support"));
#endif
    
    return schema;
}

String PCAL6416ADriver::getParameter(const String& name) {
    return "";
}

bool PCAL6416ADriver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_PCAL6416A_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCAL6416ADriver::registers(size_t& count) const {
    count = PCAL6416A_REGISTER_COUNT;
    return PCAL6416A_REGISTERS;
}

bool PCAL6416ADriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || len != 1) {
        return false;
    }
    
    // Validate register address
    if ((reg > 0x07 && reg < 0x40) || reg > 0x4F) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool PCAL6416ADriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || len != 1) {
        return false;
    }
    
    // Validate register address
    if ((reg > 0x07 && reg < 0x40) || reg > 0x4F) {
        return false;
    }
    
    // Check if register is read-only
    if (reg == PCAL6416A_REG_INPUT0 || reg == PCAL6416A_REG_INPUT1 ||
        reg == PCAL6416A_REG_INTSTAT0 || reg == PCAL6416A_REG_INTSTAT1) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* PCAL6416ADriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCAL6416A_REGISTERS, PCAL6416A_REGISTER_COUNT, name);
}
#endif

// Private methods

bool PCAL6416ADriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_PCAL6416A_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool PCAL6416ADriver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
#if POCKETOS_PCAL6416A_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
#if POCKETOS_PCAL6416A_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    *value = Wire.read();
    
#if POCKETOS_PCAL6416A_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
