#include "fdc1004_driver.h"
#include "../driver_config.h"

#if POCKETOS_FDC1004_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define FDC1004_REG_CONTROL    0x00
#define FDC1004_REG_STATUS     0x01
#define FDC1004_REG_DATA       0x02

#if POCKETOS_FDC1004_ENABLE_REGISTER_ACCESS
static const RegisterDesc FDC1004_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "DATA", 1, RegisterAccess::RO, 0x00),
};

#define FDC1004_REGISTER_COUNT (sizeof(FDC1004_REGISTERS) / sizeof(RegisterDesc))
#endif

FDC1004Driver::FDC1004Driver() : address(0), initialized(false) {}

bool FDC1004Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_FDC1004_ENABLE_LOGGING
    Logger::info("FDC1004: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_FDC1004_ENABLE_CONFIGURATION
    writeRegister(FDC1004_REG_CONTROL, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_FDC1004_ENABLE_LOGGING
    Logger::info("FDC1004: Initialized successfully");
#endif
    return true;
}

void FDC1004Driver::deinit() {
    initialized = false;
}

FDC1004Data FDC1004Driver::readData() {
    FDC1004Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t value;
    if (readRegister(FDC1004_REG_DATA, &value)) {
        data.value = value;
        data.valid = true;
    }
    
    return data;
}

CapabilitySchema FDC1004Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_FDC1004_TIER_NAME;
    return schema;
}

bool FDC1004Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool FDC1004Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_FDC1004_ENABLE_REGISTER_ACCESS
const RegisterDesc* FDC1004Driver::registers(size_t& count) const {
    count = FDC1004_REGISTER_COUNT;
    return FDC1004_REGISTERS;
}

bool FDC1004Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(FDC1004_REGISTERS, FDC1004_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool FDC1004Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(FDC1004_REGISTERS, FDC1004_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* FDC1004Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(FDC1004_REGISTERS, FDC1004_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
