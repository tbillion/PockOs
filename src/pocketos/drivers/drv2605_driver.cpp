#include "drv2605_driver.h"
#include "../driver_config.h"

#if POCKETOS_DRV2605_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define DRV2605_REG_STATUS      0x00
#define DRV2605_REG_MODE        0x01
#define DRV2605_REG_RTPIN       0x02
#define DRV2605_REG_LIBRARY     0x03
#define DRV2605_REG_WAVESEQ1    0x04
#define DRV2605_REG_GO          0x0C
#define DRV2605_REG_OVERDRIVE   0x0D
#define DRV2605_REG_FEEDBACK    0x1A
#define DRV2605_REG_CONTROL1    0x1B
#define DRV2605_REG_CONTROL2    0x1C
#define DRV2605_REG_CONTROL3    0x1D

#if POCKETOS_DRV2605_ENABLE_REGISTER_ACCESS
static const RegisterDesc DRV2605_REGISTERS[] = {
    RegisterDesc(0x00, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "MODE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "RTPIN", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "LIBRARY", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x04, "WAVESEQ1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "WAVESEQ2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "WAVESEQ3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "WAVESEQ4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "WAVESEQ5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "WAVESEQ6", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "WAVESEQ7", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "WAVESEQ8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "GO", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "OVERDRIVE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1A, "FEEDBACK", 1, RegisterAccess::RW, 0x36),
    RegisterDesc(0x1B, "CONTROL1", 1, RegisterAccess::RW, 0x93),
    RegisterDesc(0x1C, "CONTROL2", 1, RegisterAccess::RW, 0xF5),
    RegisterDesc(0x1D, "CONTROL3", 1, RegisterAccess::RW, 0xA0),
};

#define DRV2605_REGISTER_COUNT (sizeof(DRV2605_REGISTERS) / sizeof(RegisterDesc))
#endif

DRV2605Driver::DRV2605Driver() : address(0), initialized(false) {}

bool DRV2605Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_DRV2605_ENABLE_LOGGING
    Logger::info("DRV2605: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_DRV2605_ENABLE_CONFIGURATION
    writeRegister(DRV2605_REG_MODE, 0x00);
    writeRegister(DRV2605_REG_LIBRARY, 0x01);
    writeRegister(DRV2605_REG_CONTROL3, 0xA0);
#endif
    
    initialized = true;
#if POCKETOS_DRV2605_ENABLE_LOGGING
    Logger::info("DRV2605: Initialized successfully");
#endif
    return true;
}

void DRV2605Driver::deinit() {
    if (initialized) {
        writeRegister(DRV2605_REG_MODE, 0x40);
    }
    initialized = false;
}

bool DRV2605Driver::playEffect(uint8_t effect) {
    if (!initialized) {
        return false;
    }
    
    writeRegister(DRV2605_REG_WAVESEQ1, effect);
    writeRegister(DRV2605_REG_WAVESEQ1 + 1, 0x00);
    writeRegister(DRV2605_REG_GO, 0x01);
    
    return true;
}

CapabilitySchema DRV2605Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_DRV2605_TIER_NAME;
    return schema;
}

bool DRV2605Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool DRV2605Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_DRV2605_ENABLE_REGISTER_ACCESS
const RegisterDesc* DRV2605Driver::registers(size_t& count) const {
    count = DRV2605_REGISTER_COUNT;
    return DRV2605_REGISTERS;
}

bool DRV2605Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(DRV2605_REGISTERS, DRV2605_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool DRV2605Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(DRV2605_REGISTERS, DRV2605_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* DRV2605Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(DRV2605_REGISTERS, DRV2605_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
