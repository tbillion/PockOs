#include "mcp23008_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_MCP23008_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_MCP23008_ENABLE_REGISTER_ACCESS
static const RegisterDesc MCP23008_REGISTERS[] = {
    RegisterDesc(0x00, "IODIR", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x01, "IPOL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "GPINTEN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "DEFVAL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x04, "INTCON", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "IOCON", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "GPPU", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "INTF", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x08, "INTCAP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "GPIO", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "OLAT", 1, RegisterAccess::RW, 0x00),
};
#define MCP23008_REGISTER_COUNT (sizeof(MCP23008_REGISTERS) / sizeof(RegisterDesc))
#endif

MCP23008Driver::MCP23008Driver() 
    : address(0), initialized(false)
#if POCKETOS_MCP23008_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool MCP23008Driver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Set all pins as inputs by default
    if (!writeRegister(MCP23008_REG_IODIR, 0xFF)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void MCP23008Driver::deinit() {
    initialized = false;
}

bool MCP23008Driver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    uint8_t iodir;
    if (!readRegister(MCP23008_REG_IODIR, &iodir)) {
        return false;
    }
    
    if (mode == INPUT || mode == INPUT_PULLUP) {
        iodir |= (1 << pin);
    } else {
        iodir &= ~(1 << pin);
    }
    
    if (!writeRegister(MCP23008_REG_IODIR, iodir)) {
        return false;
    }
    
#if POCKETOS_MCP23008_ENABLE_CONFIGURATION
    // Enable pull-up if INPUT_PULLUP
    if (mode == INPUT_PULLUP) {
        return setPullUp(pin, true);
    }
#endif
    
    return true;
}

bool MCP23008Driver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    uint8_t gpio;
    if (!readRegister(MCP23008_REG_GPIO, &gpio)) {
        return false;
    }
    
    if (value) {
        gpio |= (1 << pin);
    } else {
        gpio &= ~(1 << pin);
    }
    
    return writeRegister(MCP23008_REG_GPIO, gpio);
}

int MCP23008Driver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 7) {
        return -1;
    }
    
    uint8_t gpio;
    if (!readRegister(MCP23008_REG_GPIO, &gpio)) {
        return -1;
    }
    
    return (gpio & (1 << pin)) ? HIGH : LOW;
}

bool MCP23008Driver::writePort(uint8_t value) {
    if (!initialized) {
        return false;
    }
    
    return writeRegister(MCP23008_REG_GPIO, value);
}

uint8_t MCP23008Driver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t gpio;
    if (!readRegister(MCP23008_REG_GPIO, &gpio)) {
        return 0;
    }
    
    return gpio;
}

#if POCKETOS_MCP23008_ENABLE_CONFIGURATION
bool MCP23008Driver::setPullUp(uint8_t pin, bool enable) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    uint8_t gppu;
    if (!readRegister(MCP23008_REG_GPPU, &gppu)) {
        return false;
    }
    
    if (enable) {
        gppu |= (1 << pin);
    } else {
        gppu &= ~(1 << pin);
    }
    
    return writeRegister(MCP23008_REG_GPPU, gppu);
}

bool MCP23008Driver::setPolarity(uint8_t pin, bool inverted) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    uint8_t ipol;
    if (!readRegister(MCP23008_REG_IPOL, &ipol)) {
        return false;
    }
    
    if (inverted) {
        ipol |= (1 << pin);
    } else {
        ipol &= ~(1 << pin);
    }
    
    return writeRegister(MCP23008_REG_IPOL, ipol);
}

bool MCP23008Driver::enableInterrupt(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    uint8_t gpinten;
    if (!readRegister(MCP23008_REG_GPINTEN, &gpinten)) {
        return false;
    }
    
    gpinten |= (1 << pin);
    return writeRegister(MCP23008_REG_GPINTEN, gpinten);
}

bool MCP23008Driver::disableInterrupt(uint8_t pin) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    uint8_t gpinten;
    if (!readRegister(MCP23008_REG_GPINTEN, &gpinten)) {
        return false;
    }
    
    gpinten &= ~(1 << pin);
    return writeRegister(MCP23008_REG_GPINTEN, gpinten);
}

uint8_t MCP23008Driver::getInterruptFlags() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t intf;
    if (!readRegister(MCP23008_REG_INTF, &intf)) {
        return 0;
    }
    
    return intf;
}

uint8_t MCP23008Driver::getInterruptCapture() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t intcap;
    if (!readRegister(MCP23008_REG_INTCAP, &intcap)) {
        return 0;
    }
    
    return intcap;
}
#endif

CapabilitySchema MCP23008Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "mcp23008";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_MCP23008_TIER_NAME;
    schema.description = "MCP23008 8-bit GPIO expander";
    
    // GPIO capabilities
    schema.capabilities.push_back(Capability("gpio_pins", "8", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    
#if POCKETOS_MCP23008_ENABLE_CONFIGURATION
    schema.capabilities.push_back(Capability("pull_up", "true", "Internal pull-up support"));
    schema.capabilities.push_back(Capability("polarity", "true", "Input polarity inversion"));
    schema.capabilities.push_back(Capability("interrupts", "true", "Interrupt on change support"));
#endif
    
    return schema;
}

String MCP23008Driver::getParameter(const String& name) {
    return "";
}

bool MCP23008Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_MCP23008_ENABLE_REGISTER_ACCESS
const RegisterDesc* MCP23008Driver::registers(size_t& count) const {
    count = MCP23008_REGISTER_COUNT;
    return MCP23008_REGISTERS;
}

bool MCP23008Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x0A || len != 1) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool MCP23008Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x0A || len != 1) {
        return false;
    }
    
    // Check if register is read-only
    if (reg == MCP23008_REG_INTF || reg == MCP23008_REG_INTCAP) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MCP23008Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MCP23008_REGISTERS, MCP23008_REGISTER_COUNT, name);
}
#endif

// Private methods

bool MCP23008Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_MCP23008_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool MCP23008Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
#if POCKETOS_MCP23008_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
#if POCKETOS_MCP23008_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    *value = Wire.read();
    
#if POCKETOS_MCP23008_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
