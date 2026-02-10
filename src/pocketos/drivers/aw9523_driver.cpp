#include "aw9523_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_AW9523_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_AW9523_ENABLE_REGISTER_ACCESS
static const RegisterDesc AW9523_REGISTERS[] = {
    RegisterDesc(0x00, "INPUT0", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x01, "INPUT1", 1, RegisterAccess::RO, 0xFF),
    RegisterDesc(0x02, "OUTPUT0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x03, "OUTPUT1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x04, "CONFIG0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x05, "CONFIG1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x06, "INT0", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x07, "INT1", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x10, "ID", 1, RegisterAccess::RO, 0x23),
    RegisterDesc(0x11, "CTL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "LED_MODE0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "LED_MODE1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x20, "DIM0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x21, "DIM1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "DIM2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x23, "DIM3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x24, "DIM4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x25, "DIM5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x26, "DIM6", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x27, "DIM7", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "DIM8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x29, "DIM9", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2A, "DIM10", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2B, "DIM11", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2C, "DIM12", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2D, "DIM13", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2E, "DIM14", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2F, "DIM15", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x7F, "SWRST", 1, RegisterAccess::WO, 0x00),
};
#define AW9523_REGISTER_COUNT (sizeof(AW9523_REGISTERS) / sizeof(RegisterDesc))
#endif

AW9523Driver::AW9523Driver() 
    : address(0), initialized(false)
#if POCKETOS_AW9523_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool AW9523Driver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Verify device ID
    uint8_t id;
    if (!readRegister(AW9523_REG_ID, &id) || id != 0x23) {
        return false;
    }
    
    // Set all pins as GPIO inputs by default
    if (!writeRegister(AW9523_REG_CONFIG0, 0xFF)) {
        return false;
    }
    if (!writeRegister(AW9523_REG_CONFIG1, 0xFF)) {
        return false;
    }
    
    // Set all pins to GPIO mode (not LED mode)
    if (!writeRegister(AW9523_REG_LED_MODE0, 0x00)) {
        return false;
    }
    if (!writeRegister(AW9523_REG_LED_MODE1, 0x00)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void AW9523Driver::deinit() {
    initialized = false;
}

uint8_t AW9523Driver::getPortReg(uint8_t pin, uint8_t reg0, uint8_t reg1) {
    return (pin < 8) ? reg0 : reg1;
}

bool AW9523Driver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, AW9523_REG_CONFIG0, AW9523_REG_CONFIG1);
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

bool AW9523Driver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, AW9523_REG_OUTPUT0, AW9523_REG_OUTPUT1);
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

int AW9523Driver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 15) {
        return -1;
    }
    
    uint8_t reg = getPortReg(pin, AW9523_REG_INPUT0, AW9523_REG_INPUT1);
    uint8_t bit = pin % 8;
    
    uint8_t input;
    if (!readRegister(reg, &input)) {
        return -1;
    }
    
    return (input & (1 << bit)) ? HIGH : LOW;
}

bool AW9523Driver::writePort(uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    if (!writeRegister(AW9523_REG_OUTPUT0, value & 0xFF)) {
        return false;
    }
    
    return writeRegister(AW9523_REG_OUTPUT1, (value >> 8) & 0xFF);
}

uint16_t AW9523Driver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t port0, port1;
    if (!readRegister(AW9523_REG_INPUT0, &port0)) {
        return 0;
    }
    if (!readRegister(AW9523_REG_INPUT1, &port1)) {
        return 0;
    }
    
    return ((uint16_t)port1 << 8) | port0;
}

#if POCKETOS_AW9523_ENABLE_CONFIGURATION
bool AW9523Driver::setLedMode(uint8_t pin, bool ledMode) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, AW9523_REG_LED_MODE0, AW9523_REG_LED_MODE1);
    uint8_t bit = pin % 8;
    
    uint8_t mode;
    if (!readRegister(reg, &mode)) {
        return false;
    }
    
    if (ledMode) {
        mode |= (1 << bit);
    } else {
        mode &= ~(1 << bit);
    }
    
    return writeRegister(reg, mode);
}

bool AW9523Driver::setLedBrightness(uint8_t pin, uint8_t brightness) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    // DIM registers start at 0x20 for P0_0
    uint8_t reg = AW9523_REG_DIM0 + pin;
    
    return writeRegister(reg, brightness);
}

bool AW9523Driver::enableInterrupt(uint8_t pin) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, AW9523_REG_INT0, AW9523_REG_INT1);
    uint8_t bit = pin % 8;
    
    uint8_t intmask;
    if (!readRegister(reg, &intmask)) {
        return false;
    }
    
    intmask &= ~(1 << bit);  // Clear bit to enable interrupt
    return writeRegister(reg, intmask);
}

bool AW9523Driver::disableInterrupt(uint8_t pin) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    uint8_t reg = getPortReg(pin, AW9523_REG_INT0, AW9523_REG_INT1);
    uint8_t bit = pin % 8;
    
    uint8_t intmask;
    if (!readRegister(reg, &intmask)) {
        return false;
    }
    
    intmask |= (1 << bit);  // Set bit to disable interrupt
    return writeRegister(reg, intmask);
}

uint16_t AW9523Driver::getInterruptStatus() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t int0, int1;
    if (!readRegister(AW9523_REG_INT0, &int0)) {
        return 0;
    }
    if (!readRegister(AW9523_REG_INT1, &int1)) {
        return 0;
    }
    
    return ((uint16_t)int1 << 8) | int0;
}

bool AW9523Driver::softReset() {
    if (!initialized) {
        return false;
    }
    
    return writeRegister(AW9523_REG_SWRST, 0x00);
}
#endif

CapabilitySchema AW9523Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "aw9523";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_AW9523_TIER_NAME;
    schema.description = "AW9523 16-channel GPIO + LED driver";
    
    schema.capabilities.push_back(Capability("gpio_pins", "16", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    
#if POCKETOS_AW9523_ENABLE_CONFIGURATION
    schema.capabilities.push_back(Capability("led_mode", "true", "LED driver mode"));
    schema.capabilities.push_back(Capability("pwm_dimming", "true", "PWM LED dimming (256 levels)"));
    schema.capabilities.push_back(Capability("interrupts", "true", "Interrupt support"));
#endif
    
    return schema;
}

String AW9523Driver::getParameter(const String& name) {
    return "";
}

bool AW9523Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_AW9523_ENABLE_REGISTER_ACCESS
const RegisterDesc* AW9523Driver::registers(size_t& count) const {
    count = AW9523_REGISTER_COUNT;
    return AW9523_REGISTERS;
}

bool AW9523Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || len != 1) {
        return false;
    }
    
    // Validate register address
    if ((reg > 0x07 && reg < 0x10) || 
        (reg > 0x13 && reg < 0x20) || 
        (reg > 0x2F && reg < 0x7F) ||
        reg > 0x7F) {
        return false;
    }
    
    // SWRST is write-only
    if (reg == AW9523_REG_SWRST) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool AW9523Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || len != 1) {
        return false;
    }
    
    // Validate register address
    if ((reg > 0x07 && reg < 0x10) || 
        (reg > 0x13 && reg < 0x20) || 
        (reg > 0x2F && reg < 0x7F) ||
        reg > 0x7F) {
        return false;
    }
    
    // Check if register is read-only
    if (reg == AW9523_REG_INPUT0 || reg == AW9523_REG_INPUT1 || reg == AW9523_REG_ID) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* AW9523Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(AW9523_REGISTERS, AW9523_REGISTER_COUNT, name);
}
#endif

// Private methods

bool AW9523Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_AW9523_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool AW9523Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
#if POCKETOS_AW9523_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
#if POCKETOS_AW9523_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    *value = Wire.read();
    
#if POCKETOS_AW9523_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
