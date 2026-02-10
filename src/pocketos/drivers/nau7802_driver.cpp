#include "nau7802_driver.h"
#include "../driver_config.h"

#if POCKETOS_NAU7802_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define NAU7802_REG_PU_CTRL       0x00
#define NAU7802_REG_CTRL1         0x01
#define NAU7802_REG_CTRL2         0x02
#define NAU7802_REG_OCAL1_B2      0x03
#define NAU7802_REG_ADC_B2        0x12
#define NAU7802_REG_ADC_B1        0x13
#define NAU7802_REG_ADC_B0        0x14
#define NAU7802_REG_DEVICE_REV    0x1F

#if POCKETOS_NAU7802_ENABLE_REGISTER_ACCESS
static const RegisterDesc NAU7802_REGISTERS[] = {
    RegisterDesc(0x00, "PU_CTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "CTRL1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "CTRL2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "OCAL1_B2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "ADC_B2", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x13, "ADC_B1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x14, "ADC_B0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1F, "DEVICE_REV", 1, RegisterAccess::RO, 0x0F),
};

#define NAU7802_REGISTER_COUNT (sizeof(NAU7802_REGISTERS) / sizeof(RegisterDesc))
#endif

NAU7802Driver::NAU7802Driver() : address(0), initialized(false) {}

bool NAU7802Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_NAU7802_ENABLE_LOGGING
    Logger::info("NAU7802: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t rev = 0;
    if (!readRegister(NAU7802_REG_DEVICE_REV, &rev)) {
#if POCKETOS_NAU7802_ENABLE_LOGGING
        Logger::error("NAU7802: Failed to read device revision");
#endif
        return false;
    }
    
#if POCKETOS_NAU7802_ENABLE_CONFIGURATION
    writeRegister(NAU7802_REG_PU_CTRL, 0x06);
    delay(10);
    writeRegister(NAU7802_REG_CTRL1, 0x00);
    writeRegister(NAU7802_REG_CTRL2, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_NAU7802_ENABLE_LOGGING
    Logger::info("NAU7802: Initialized successfully");
#endif
    return true;
}

void NAU7802Driver::deinit() {
    if (initialized) {
        writeRegister(NAU7802_REG_PU_CTRL, 0x00);
    }
    initialized = false;
}

NAU7802Data NAU7802Driver::readData() {
    NAU7802Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t b2, b1, b0;
    if (readRegister(NAU7802_REG_ADC_B2, &b2) &&
        readRegister(NAU7802_REG_ADC_B1, &b1) &&
        readRegister(NAU7802_REG_ADC_B0, &b0)) {
        data.adcValue = ((int32_t)b2 << 16) | ((int32_t)b1 << 8) | b0;
        if (data.adcValue & 0x800000) {
            data.adcValue |= 0xFF000000;
        }
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema NAU7802Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_NAU7802_TIER_NAME;
    return schema;
}

bool NAU7802Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool NAU7802Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        *value = Wire.read();
        return true;
    }
    return false;
}

#if POCKETOS_NAU7802_ENABLE_REGISTER_ACCESS
const RegisterDesc* NAU7802Driver::registers(size_t& count) const {
    count = NAU7802_REGISTER_COUNT;
    return NAU7802_REGISTERS;
}

bool NAU7802Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(NAU7802_REGISTERS, NAU7802_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool NAU7802Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(NAU7802_REGISTERS, NAU7802_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* NAU7802Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(NAU7802_REGISTERS, NAU7802_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
