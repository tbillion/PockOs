#include "mpr121_driver.h"
#include "../driver_config.h"

#if POCKETOS_MPR121_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define MPR121_REG_TOUCHSTATUS_L  0x00
#define MPR121_REG_FILTDATA_0L    0x04
#define MPR121_REG_MHDR           0x2B
#define MPR121_REG_ECR            0x5E
#define MPR121_REG_SOFTRESET      0x80

MPR121Driver::MPR121Driver() : address(0), initialized(false) {}

bool MPR121Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MPR121_ENABLE_LOGGING
    Logger::info("MPR121: Initializing at address 0x" + String(address, HEX));
#endif
    
    writeRegister(MPR121_REG_SOFTRESET, 0x63);
    delay(10);
    
    writeRegister(MPR121_REG_ECR, 0x00);
    
#if POCKETOS_MPR121_ENABLE_CONFIGURATION
    writeRegister(MPR121_REG_MHDR, 0x01);
    writeRegister(0x2C, 0x01);
    writeRegister(0x2D, 0x00);
    writeRegister(0x2E, 0x00);
    writeRegister(0x2F, 0x01);
    writeRegister(0x30, 0x01);
    writeRegister(0x31, 0xFF);
    writeRegister(0x32, 0x02);
    
    for (uint8_t i = 0; i < 12; i++) {
        writeRegister(0x41 + 2 * i, 0x0F);
        writeRegister(0x42 + 2 * i, 0x0A);
    }
    
    writeRegister(0x5D, 0x04);
#endif
    
    writeRegister(MPR121_REG_ECR, 0x8F);
    
    initialized = true;
#if POCKETOS_MPR121_ENABLE_LOGGING
    Logger::info("MPR121: Initialized successfully");
#endif
    return true;
}

void MPR121Driver::deinit() {
    if (initialized) {
        writeRegister(MPR121_REG_ECR, 0x00);
    }
    initialized = false;
}

MPR121Data MPR121Driver::readData() {
    MPR121Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint16_t touchStatus = 0;
    if (!readWord(MPR121_REG_TOUCHSTATUS_L, &touchStatus)) {
        return data;
    }
    
    data.touched = touchStatus & 0x0FFF;
    
    for (int i = 0; i < 12; i++) {
        readWord(MPR121_REG_FILTDATA_0L + i * 2, &data.filtered[i]);
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema MPR121Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "mpr121";
    schema.tier = POCKETOS_MPR121_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("touched", "Touch status bits", "bitmask", "0-4095"));
    for (int i = 0; i < 12; i++) {
        schema.outputs.push_back(OutputDesc("ch" + String(i), "Channel " + String(i), "10-bit", "0-1023"));
    }
    
    return schema;
}

String MPR121Driver::getParameter(const String& name) {
    return "";
}

bool MPR121Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool MPR121Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
        return false;
    }
    
    *value = Wire.read();
    return true;
}

bool MPR121Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MPR121Driver::readWord(uint8_t reg, uint16_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)2) != 2) {
        return false;
    }
    
    uint8_t lsb = Wire.read();
    uint8_t msb = Wire.read();
    *value = (msb << 8) | lsb;
    return true;
}

#if POCKETOS_MPR121_ENABLE_REGISTER_ACCESS
static const RegisterDesc MPR121_REGISTERS[] = {
    RegisterDesc(0x00, "TOUCHSTATUS_L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "TOUCHSTATUS_H", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x04, "FILTDATA_0L", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2B, "MHDR", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x5D, "AFE_CONFIG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5E, "ECR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x80, "SOFTRESET", 1, RegisterAccess::WO, 0x00),
};

#define MPR121_REGISTER_COUNT (sizeof(MPR121_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* MPR121Driver::registers(size_t& count) const {
    count = MPR121_REGISTER_COUNT;
    return MPR121_REGISTERS;
}

bool MPR121Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf);
}

bool MPR121Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MPR121Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < MPR121_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(MPR121_REGISTERS[i].name)) {
            return &MPR121_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
