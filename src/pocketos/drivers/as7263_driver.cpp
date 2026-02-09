#include "as7263_driver.h"
#include "../driver_config.h"

#if POCKETOS_AS7263_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// AS7263 Virtual Registers
#define AS7263_VREG_HW_VERSION    0x00
#define AS7263_VREG_CONTROL       0x04
#define AS7263_VREG_INT_TIME      0x05
#define AS7263_VREG_DEVICE_TEMP   0x06
#define AS7263_VREG_LED_CONTROL   0x07
#define AS7263_VREG_R_HIGH        0x08
#define AS7263_VREG_R_LOW         0x09
#define AS7263_VREG_S_HIGH        0x0A
#define AS7263_VREG_S_LOW         0x0B
#define AS7263_VREG_T_HIGH        0x0C
#define AS7263_VREG_T_LOW         0x0D
#define AS7263_VREG_U_HIGH        0x0E
#define AS7263_VREG_U_LOW         0x0F
#define AS7263_VREG_V_HIGH        0x10
#define AS7263_VREG_V_LOW         0x11
#define AS7263_VREG_W_HIGH        0x12
#define AS7263_VREG_W_LOW         0x13

#if POCKETOS_AS7263_ENABLE_REGISTER_ACCESS
static const RegisterDesc AS7263_REGISTERS[] = {
    RegisterDesc(0x00, "HW_VERSION", 1, RegisterAccess::RO, 0x3E),
    RegisterDesc(0x04, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "INT_TIME", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "DEVICE_TEMP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x07, "LED_CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "R_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "R_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "S_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0B, "S_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "T_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "T_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0E, "U_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0F, "U_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x10, "V_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x11, "V_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x12, "W_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x13, "W_LOW", 1, RegisterAccess::RO, 0x00),
};

#define AS7263_REGISTER_COUNT (sizeof(AS7263_REGISTERS) / sizeof(RegisterDesc))
#endif

AS7263Driver::AS7263Driver() : address(0), initialized(false) {}

bool AS7263Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AS7263_ENABLE_LOGGING
    Logger::info("AS7263: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t hwVersion = 0;
    if (!readRegister(AS7263_VREG_HW_VERSION, &hwVersion)) {
#if POCKETOS_AS7263_ENABLE_LOGGING
        Logger::error("AS7263: Failed to read hardware version");
#endif
        return false;
    }
    
#if POCKETOS_AS7263_ENABLE_CONFIGURATION
    writeRegister(AS7263_VREG_CONTROL, 0x00);
    writeRegister(AS7263_VREG_INT_TIME, 0xFF);
#endif
    
    initialized = true;
#if POCKETOS_AS7263_ENABLE_LOGGING
    Logger::info("AS7263: Initialized successfully");
#endif
    return true;
}

void AS7263Driver::deinit() {
    initialized = false;
}

AS7263Data AS7263Driver::readData() {
    AS7263Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t raw[12];
    for (int i = 0; i < 12; i++) {
        if (!readRegister(AS7263_VREG_R_HIGH + i, &raw[i])) {
            return data;
        }
    }
    
    data.r = (raw[0] << 8) | raw[1];
    data.s = (raw[2] << 8) | raw[3];
    data.t = (raw[4] << 8) | raw[5];
    data.u = (raw[6] << 8) | raw[7];
    data.v = (raw[8] << 8) | raw[9];
    data.w = (raw[10] << 8) | raw[11];
    data.valid = true;
    
    return data;
}

CapabilitySchema AS7263Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_AS7263_TIER_NAME;
    return schema;
}

bool AS7263Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool AS7263Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_AS7263_ENABLE_REGISTER_ACCESS
const RegisterDesc* AS7263Driver::registers(size_t& count) const {
    count = AS7263_REGISTER_COUNT;
    return AS7263_REGISTERS;
}

bool AS7263Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AS7263_REGISTERS, AS7263_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool AS7263Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AS7263_REGISTERS, AS7263_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* AS7263Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(AS7263_REGISTERS, AS7263_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
