#include "pcf8574_driver.h"
#include "../core/i2c_interface.h"
#if POCKETOS_PCF8574_ENABLE_LOGGING
#include "../core/logging.h"
#endif

namespace PocketOS {

#if POCKETOS_PCF8574_ENABLE_REGISTER_ACCESS
static const RegisterDesc PCF8574_REGISTERS[] = {
    RegisterDesc(0x00, "PORT", 1, RegisterAccess::RW, 0xFF),
};
#define PCF8574_REGISTER_COUNT (sizeof(PCF8574_REGISTERS) / sizeof(RegisterDesc))
#endif

PCF8574Driver::PCF8574Driver() 
    : address(0), initialized(false), outputState(0xFF)
#if POCKETOS_PCF8574_ENABLE_LOGGING
    , operationCount(0), errorCount(0)
#endif
{
}

bool PCF8574Driver::init(uint8_t i2cAddress) {
    if (!supportsAddress(i2cAddress)) {
        return false;
    }
    
    address = i2cAddress;
    
    // Set all pins high (default state for quasi-bidirectional)
    outputState = 0xFF;
    if (!writeByte(outputState)) {
        return false;
    }
    
    initialized = true;
    return true;
}

void PCF8574Driver::deinit() {
    initialized = false;
}

bool PCF8574Driver::pinMode(uint8_t pin, uint8_t mode) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    // For PCF8574, to use a pin as input, write 1 to it
    if (mode == INPUT || mode == INPUT_PULLUP) {
        outputState |= (1 << pin);
    }
    // Output pins can be set to 0 or 1 as needed
    
    return writeByte(outputState);
}

bool PCF8574Driver::digitalWrite(uint8_t pin, bool value) {
    if (!initialized || pin > 7) {
        return false;
    }
    
    if (value) {
        outputState |= (1 << pin);
    } else {
        outputState &= ~(1 << pin);
    }
    
    return writeByte(outputState);
}

int PCF8574Driver::digitalRead(uint8_t pin) {
    if (!initialized || pin > 7) {
        return -1;
    }
    
    uint8_t value;
    if (!readByte(&value)) {
        return -1;
    }
    
    return (value & (1 << pin)) ? HIGH : LOW;
}

bool PCF8574Driver::writePort(uint8_t value) {
    if (!initialized) {
        return false;
    }
    
    outputState = value;
    return writeByte(outputState);
}

uint8_t PCF8574Driver::readPort() {
    if (!initialized) {
        return 0;
    }
    
    uint8_t value;
    if (!readByte(&value)) {
        return 0;
    }
    
    return value;
}

CapabilitySchema PCF8574Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "pcf8574";
    schema.deviceClass = "gpio_expander";
    schema.tier = POCKETOS_PCF8574_TIER_NAME;
    schema.description = "PCF8574 8-bit quasi-bidirectional I/O";
    
    schema.capabilities.push_back(Capability("gpio_pins", "8", "Number of GPIO pins"));
    schema.capabilities.push_back(Capability("digital_out", "true", "Digital output support"));
    schema.capabilities.push_back(Capability("digital_in", "true", "Digital input support"));
    schema.capabilities.push_back(Capability("quasi_bidirectional", "true", "Quasi-bidirectional I/O"));
    
    return schema;
}

String PCF8574Driver::getParameter(const String& name) {
    return "";
}

bool PCF8574Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_PCF8574_ENABLE_REGISTER_ACCESS
const RegisterDesc* PCF8574Driver::registers(size_t& count) const {
    count = PCF8574_REGISTER_COUNT;
    return PCF8574_REGISTERS;
}

bool PCF8574Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x00 || len != 1) {
        return false;
    }
    
    return readByte(buf);
}

bool PCF8574Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0x00 || len != 1) {
        return false;
    }
    
    outputState = buf[0];
    return writeByte(outputState);
}

const RegisterDesc* PCF8574Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(PCF8574_REGISTERS, PCF8574_REGISTER_COUNT, name);
}
#endif

// Private methods

bool PCF8574Driver::writeByte(uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(value);
    uint8_t result = Wire.endTransmission();
    
#if POCKETOS_PCF8574_ENABLE_LOGGING
    if (result != 0) {
        errorCount++;
    }
    operationCount++;
#endif
    
    return result == 0;
}

bool PCF8574Driver::readByte(uint8_t* value) {
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
#if POCKETOS_PCF8574_ENABLE_LOGGING
        errorCount++;
#endif
        return false;
    }
    
    *value = Wire.read();
    
#if POCKETOS_PCF8574_ENABLE_LOGGING
    operationCount++;
#endif
    
    return true;
}

} // namespace PocketOS
