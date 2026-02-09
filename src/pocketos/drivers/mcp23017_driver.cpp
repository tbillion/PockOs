#include "mcp23017_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_MCP23017_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_MCP23017_ENABLE_REGISTER_ACCESS
static const RegisterDesc MCP23017_REGISTERS[] = {
    RegisterDesc(0x00, "IODIRA", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x01, "IODIRB", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x02, "IPOLA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "IPOLB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x04, "GPINTENA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "GPINTENB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "DEFVALA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "DEFVALB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "INTCONA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "INTCONB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "IOCON", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "IOCON", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "GPPUA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "GPPUB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "INTFA", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0F, "INTFB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x10, "INTCAPA", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x11, "INTCAPB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x12, "GPIOA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "GPIOB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x14, "OLATA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x15, "OLATB", 1, RegisterAccess::RW, 0x00),
};
#define MCP23017_REGISTER_COUNT (sizeof(MCP23017_REGISTERS) / sizeof(RegisterDesc))
#endif

MCP23017Driver::MCP23017Driver() 
    : address(0), initialized(false)
#if POCKETOS_MCP23017_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool MCP23017Driver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Set all pins as inputs by default
    if (!writeRegister(MCP23017_REG_IODIRA, 0xFF)) {
        return false;
    }
    if (!writeRegister(MCP23017_REG_IODIRB, 0xFF)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void MCP23017Driver::deinit() {
    initialized = false;
}

uint8_t MCP23017Driver::getPortReg(uint8_t pin, uint8_t regA, uint8_t regB) {
    return (pin < 8) ? regA : regB;
}

bool MCP23017Driver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, MCP23017_REG_IODIRA, MCP23017_REG_IODIRB);
    uint8_t bit = pin % 8;
    
    uint8_t iodir;
    if (!readRegister(reg, &iodir)) {
        return false;
    }
    
    if (mode == INPUT || mode == INPUT_PULLUP) {
        iodir |= (1 << bit);
    } else {
        iodir &= ~(1 << bit);
    }
    
    if (!writeRegister(reg, iodir)) {
        return false;
    }
    
#if POCKETOS_MCP23017_ENABLE_CONFIGURATION
    if (mode == INPUT_PULLUP) {
        return setPullUp(pin, true);
    }
#endif
    
    return true;
}

bool MCP23017Driver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, MCP23017_REG_GPIOA, MCP23017_REG_GPIOB);
    uint8_t bit = pin % 8;
    
    uint8_t gpio;
    if (!readRegister(reg, &gpio)) {
        return false;
    }
    
    if (value) {
        gpio |= (1 << bit);
    } else {
        gpio &= ~(1 << bit);
    }
    
    return writeRegister(reg, gpio);
}

int MCP23017Driver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 15) {
        return -1;
    }
    
    uint8_t reg = getPortReg(pin, MCP23017_REG_GPIOA, MCP23017_REG_GPIOB);
    uint8_t bit = pin % 8;
    
    uint8_t gpio;
    if (!readRegister(reg, &gpio)) {
        return -1;
    }
    
    return (gpio & (1 << bit)) ? HIGH : LOW;
}

bool MCP23017Driver::writePort(uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    if (!writeRegister(MCP23017_REG_GPIOA, value & 0xFF)) {
        return false;
    }
    
    return writeRegister(MCP23017_REG_GPIOB, (value >> 8) & 0xFF);
}

uint16_t MCP23017Driver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t portA, portB;
    if (!readRegister(MCP23017_REG_GPIOA, &portA)) {
        return 0;
    }
    if (!readRegister(MCP23017_REG_GPIOB, &portB)) {
        return 0;
    }
    
    return ((uint16_t)portB << 8) | portA;
}

bool MCP23017Driver::writePortA(uint8_t value) {
    return initialized && writeRegister(MCP23017_REG_GPIOA, value);
}

bool MCP23017Driver::writePortB(uint8_t value) {
    return initialized && writeRegister(MCP23017_REG_GPIOB, value);
}

uint8_t MCP23017Driver::readPortA() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t value;
    if (!readRegister(MCP23017_REG_GPIOA, &value)) {
        return 0;
    }
    return value;
}

uint8_t MCP23017Driver::readPortB() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t value;
    if (!readRegister(MCP23017_REG_GPIOB, &value)) {
        return 0;
    }
    return value;
}

#if POCKETOS_MCP23017_ENABLE_CONFIGURATION
bool MCP23017Driver::setPullUp(uint8_t pin, bool enable) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, MCP23017_REG_GPPUA, MCP23017_REG_GPPUB);
    uint8_t bit = pin % 8;
    
    uint8_t gppu;
    if (!readRegister(reg, &gppu)) {
        return false;
    }
    
    if (enable) {
        gppu |= (1 << bit);
    } else {
        gppu &= ~(1 << bit);
    }
    
    return writeRegister(reg, gppu);
}

bool MCP23017Driver::setPolarity(uint8_t pin, bool inverted) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, MCP23017_REG_IPOLA, MCP23017_REG_IPOLB);
    uint8_t bit = pin % 8;
    
    uint8_t ipol;
    if (!readRegister(reg, &ipol)) {
        return false;
    }
    
    if (inverted) {
        ipol |= (1 << bit);
    } else {
        ipol &= ~(1 << bit);
    }
    
    return writeRegister(reg, ipol);
}

bool MCP23017Driver::enableInterrupt(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, MCP23017_REG_GPINTENA, MCP23017_REG_GPINTENB);
    uint8_t bit = pin % 8;
    
    uint8_t gpinten;
    if (!readRegister(reg, &gpinten)) {
        return false;
    }
    
    gpinten |= (1 << bit);
    return writeRegister(reg, gpinten);
}

bool MCP23017Driver::disableInterrupt(uint8_t pin) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, MCP23017_REG_GPINTENA, MCP23017_REG_GPINTENB);
    uint8_t bit = pin % 8;
    
    uint8_t gpinten;
    if (!readRegister(reg, &gpinten)) {
        return false;
    }
    
    gpinten &= ~(1 << bit);
    return writeRegister(reg, gpinten);
}

uint16_t MCP23017Driver::getInterruptFlags() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t intfA, intfB;
    if (!readRegister(MCP23017_REG_INTFA, &intfA)) {
        return 0;
    }
    if (!readRegister(MCP23017_REG_INTFB, &intfB)) {
        return 0;
    }
    
    return ((uint16_t)intfB << 8) | intfA;
}

uint16_t MCP23017Driver::getInterruptCapture() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t intcapA, intcapB;
    if (!readRegister(MCP23017_REG_INTCAPA, &intcapA)) {
        return 0;
    }
    if (!readRegister(MCP23017_REG_INTCAPB, &intcapB)) {
        return 0;
    }
    
    return ((uint16_t)intcapB << 8) | intcapA;
}
#endif

CapabilitySchema MCP23017Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "mcp23017";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_MCP23017_TIER_NAME;
    schema.description = "MCP23017 16-bit GPIO expander";
    
    schema.capabilities.push_back(Capability("gpio_pins", "16", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    
#if POCKETOS_MCP23017_ENABLE_CONFIGURATION
    schema.capabilities.push_back(Capability("pull_up", "true", "Internal pull-up support"));
    schema.capabilities.push_back(Capability("polarity", "true", "Input polarity inversion"));
    schema.capabilities.push_back(Capability("interrupts", "true", "Interrupt on change support"));
#endif
    
    return schema;
}

String MCP23017Driver::getParameter(const String& name) {
    return "";
}

bool MCP23017Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_MCP23017_ENABLE_REGISTER_ACCESS
const RegisterDesc* MCP23017Driver::registers(size_t& count) const {
    count = MCP23017_REGISTER_COUNT;
    return MCP23017_REGISTERS;
}

bool MCP23017Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x15 || len != 1) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool MCP23017Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x15 || len != 1) {
        return false;
    }
    
    // Check if register is read-only
    if (reg >= 0x0E && reg <= 0x11) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MCP23017Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MCP23017_REGISTERS, MCP23017_REGISTER_COUNT, name);
}
#endif

// Private methods

bool MCP23017Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_MCP23017_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool MCP23017Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
#if POCKETOS_MCP23017_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
#if POCKETOS_MCP23017_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    *value = Wire.read();
    
#if POCKETOS_MCP23017_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
