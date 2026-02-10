#include "pcf8575_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_PCF8575_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_PCF8575_ENABLE_REGISTER_ACCESS
static const RegisterDesc PCF8575_REGISTERS[] = {
    RegisterDesc(0x00, "PORT_LOW", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x01, "PORT_HIGH", 1, RegisterAccess::RW, 0xFF),
};
#define PCF8575_REGISTER_COUNT (sizeof(PCF8575_REGISTERS) / sizeof(RegisterDesc))
#endif

PCF8575Driver::PCF8575Driver() 
    : address(0), initialized(false), outputState(0xFFFF)
#if POCKETOS_PCF8575_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool PCF8575Driver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Set all pins high (default state for quasi-bidirectional)
    outputState = 0xFFFF;
    if (!writeWord(outputState)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void PCF8575Driver::deinit() {
    initialized = false;
}

bool PCF8575Driver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    // For PCF8575, to use a pin as input, write 1 to it
    if (mode == INPUT || mode == INPUT_PULLUP) {
        outputState |= (1 << pin);
    }
    
    return writeWord(outputState);
}

bool PCF8575Driver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 15) {
        return false;
    }
    
    if (value) {
        outputState |= (1 << pin);
    } else {
        outputState &= ~(1 << pin);
    }
    
    return writeWord(outputState);
}

int PCF8575Driver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 15) {
        return -1;
    }
    
    uint16_t value;
    if (!readWord(&value)) {
        return -1;
    }
    
    return (value & (1 << pin)) ? HIGH : LOW;
}

bool PCF8575Driver::writePort(uint16_t value) {
    if (!initialized) {
        return false;
    }
    
    outputState = value;
    return writeWord(outputState);
}

uint16_t PCF8575Driver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint16_t value;
    if (!readWord(&value)) {
        return 0;
    }
    
    return value;
}

CapabilitySchema PCF8575Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "pcf8575";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_PCF8575_TIER_NAME;
    schema.description = "PCF8575 16-bit quasi-bidirectional I/O";
    
    schema.capabilities.push_back(Capability("gpio_pins", "16", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    schema.capabilities.push_back(Capability("quasi_bidirectional", "true", "Quasi-bidirectional I/O"));
    
    return schema;
}

String PCF8575Driver::getParameter(const String& name) {
    return "";
}

bool PCF8575Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_PCF8575_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCF8575Driver::registers(size_t& count) const {
    count = PCF8575_REGISTER_COUNT;
    return PCF8575_REGISTERS;
}

bool PCF8575Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x01 || len != 1) {
        return false;
    }
    
    uint16_t value;
    if (!readWord(&value)) {
        return false;
    }
    
    if (reg == 0x00) {
        buf[0] = value & 0xFF;
    } else {
        buf[0] = (value >> 8) & 0xFF;
    }
    
    return true;
}

bool PCF8575Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x01 || len != 1) {
        return false;
    }
    
    if (reg == 0x00) {
        outputState = (outputState & 0xFF00) | buf[0];
    } else {
        outputState = (outputState & 0x00FF) | ((uint16_t)buf[0] << 8);
    }
    
    return writeWord(outputState);
}

const RegisterDesc* PCF8575Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCF8575_REGISTERS, PCF8575_REGISTER_COUNT, name);
}
#endif

// Private methods

bool PCF8575Driver::writeWord(uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(value & 0xFF);
    Wire.write((value >> 8) & 0xFF);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_PCF8575_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool PCF8575Driver::readWord(uint16_t* value) {
    if (Wire.requestFrom(address, (uint8_t)2) != 2) {
#if POCKETOS_PCF8575_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    uint8_t low = Wire.read();
    uint8_t high = Wire.read();
    *value = ((uint16_t)high << 8) | low;
    
#if POCKETOS_PCF8575_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
