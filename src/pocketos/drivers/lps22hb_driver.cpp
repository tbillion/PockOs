#include "lps22hb_driver.h"
#include "../driver_config.h"
#if POCKETOS_LPS22HB_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define LPS22HB_REG_WHO_AM_I   0x0F
#define LPS22HB_REG_CTRL_REG1  0x10
#define LPS22HB_REG_CTRL_REG2  0x11
#define LPS22HB_REG_STATUS     0x27
#define LPS22HB_REG_PRESS_OUT  0x28
#define LPS22HB_REG_TEMP_OUT   0x2B
#define LPS22HB_CHIP_ID        0xB1

LPS22HBDriver::LPS22HBDriver() : address(0), initialized(false) {
}

bool LPS22HBDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    uint8_t chipId = 0;
    if (!readRegister(LPS22HB_REG_WHO_AM_I, &chipId) || chipId != LPS22HB_CHIP_ID) {
#if POCKETOS_LPS22HB_ENABLE_LOGGING
        Logger::error("LPS22HB: Invalid chip ID");
#endif
        return false;
    }
    
    writeRegister(LPS22HB_REG_CTRL_REG1, 0x10);
    
    initialized = true;
#if POCKETOS_LPS22HB_ENABLE_LOGGING
    Logger::info("LPS22HB: Initialized");
#endif
    return true;
}

void LPS22HBDriver::deinit() {
    if (initialized) {
        writeRegister(LPS22HB_REG_CTRL_REG1, 0x00);
    }
    initialized = false;
}

LPS22HBData LPS22HBDriver::readData() {
    LPS22HBData data;
    if (!initialized) return data;
    
    uint8_t press_buffer[3];
    uint8_t temp_buffer[2];
    
    if (!readRegisters(LPS22HB_REG_PRESS_OUT, press_buffer, 3)) return data;
    if (!readRegisters(LPS22HB_REG_TEMP_OUT, temp_buffer, 2)) return data;
    
    int32_t press_raw = ((int32_t)press_buffer[2] << 16) | ((int32_t)press_buffer[1] << 8) | press_buffer[0];
    int16_t temp_raw = ((int16_t)temp_buffer[1] << 8) | temp_buffer[0];
    
    data.pressure = press_raw / 4096.0f;
    data.temperature = temp_raw / 100.0f;
    data.valid = true;
    
    return data;
}

CapabilitySchema LPS22HBDriver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x5C", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "lps22hb", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_LPS22HB_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addCommand("read", "");
    return schema;
}

String LPS22HBDriver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "lps22hb";
    if (name == "tier") return POCKETOS_LPS22HB_TIER_NAME;
    return "";
}

bool LPS22HBDriver::setParameter(const String& name, const String& value) {
    return false;
}

bool LPS22HBDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool LPS22HBDriver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        *value = Wire.read();
        return true;
    }
    return false;
}

bool LPS22HBDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg | 0x80);
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom(address, (uint8_t)len);
    size_t count = 0;
    while (Wire.available() && count < len) {
        buffer[count++] = Wire.read();
    }
    return (count == len);
}

#if POCKETOS_LPS22HB_ENABLE_REGISTER_ACCESS
const RegisterDesc* LPS22HBDriver::registers(size_t& count) const {
    static const RegisterDesc LPS22HB_REGISTERS[] = {
        RegisterDesc(0x0F, "WHO_AM_I", 1, RegisterAccess::RO, 0xB1),
        RegisterDesc(0x10, "CTRL_REG1", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x11, "CTRL_REG2", 1, RegisterAccess::RW, 0x10),
        RegisterDesc(0x27, "STATUS", 1, RegisterAccess::RO, 0x00),
        RegisterDesc(0x28, "PRESS_OUT_XL", 1, RegisterAccess::RO, 0x00),
        RegisterDesc(0x2B, "TEMP_OUT_L", 1, RegisterAccess::RO, 0x00),
    };
    count = sizeof(LPS22HB_REGISTERS) / sizeof(RegisterDesc);
    return LPS22HB_REGISTERS;
}

bool LPS22HBDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    return readRegister((uint8_t)reg, buf);
}

bool LPS22HBDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* LPS22HBDriver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
