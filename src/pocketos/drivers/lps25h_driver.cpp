#include "lps25h_driver.h"
#include "../driver_config.h"
#if POCKETOS_LPS25H_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define LPS25H_REG_WHO_AM_I    0x0F
#define LPS25H_REG_CTRL_REG1   0x20
#define LPS25H_REG_CTRL_REG2   0x21
#define LPS25H_REG_STATUS      0x27
#define LPS25H_REG_PRESS_OUT   0x28
#define LPS25H_REG_TEMP_OUT    0x2B
#define LPS25H_CHIP_ID         0xBD

LPS25HDriver::LPS25HDriver() : address(0), initialized(false) {
}

bool LPS25HDriver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    uint8_t chipId = 0;
    if (!readRegister(LPS25H_REG_WHO_AM_I, &chipId) || chipId != LPS25H_CHIP_ID) {
#if POCKETOS_LPS25H_ENABLE_LOGGING
        Logger::error("LPS25H: Invalid chip ID");
#endif
        return false;
    }
    
    writeRegister(LPS25H_REG_CTRL_REG1, 0xB0);
    
    initialized = true;
#if POCKETOS_LPS25H_ENABLE_LOGGING
    Logger::info("LPS25H: Initialized");
#endif
    return true;
}

void LPS25HDriver::deinit() {
    if (initialized) {
        writeRegister(LPS25H_REG_CTRL_REG1, 0x00);
    }
    initialized = false;
}

LPS25HData LPS25HDriver::readData() {
    LPS25HData data;
    if (!initialized) return data;
    
    uint8_t press_buffer[3];
    uint8_t temp_buffer[2];
    
    if (!readRegisters(LPS25H_REG_PRESS_OUT, press_buffer, 3)) return data;
    if (!readRegisters(LPS25H_REG_TEMP_OUT, temp_buffer, 2)) return data;
    
    int32_t press_raw = ((int32_t)press_buffer[2] << 16) | ((int32_t)press_buffer[1] << 8) | press_buffer[0];
    int16_t temp_raw = ((int16_t)temp_buffer[1] << 8) | temp_buffer[0];
    
    data.pressure = press_raw / 4096.0f;
    data.temperature = 42.5f + (temp_raw / 480.0f);
    data.valid = true;
    
    return data;
}

CapabilitySchema LPS25HDriver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x5C", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "lps25h", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_LPS25H_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addCommand("read", "");
    return schema;
}

String LPS25HDriver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "lps25h";
    if (name == "tier") return POCKETOS_LPS25H_TIER_NAME;
    return "";
}

bool LPS25HDriver::setParameter(const String& name, const String& value) {
    return false;
}

bool LPS25HDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool LPS25HDriver::readRegister(uint8_t reg, uint8_t* value) {
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

bool LPS25HDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
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

#if POCKETOS_LPS25H_ENABLE_REGISTER_ACCESS
const RegisterDesc* LPS25HDriver::registers(size_t& count) const {
    static const RegisterDesc LPS25H_REGISTERS[] = {
        RegisterDesc(0x0F, "WHO_AM_I", 1, RegisterAccess::RO, 0xBD),
        RegisterDesc(0x20, "CTRL_REG1", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x21, "CTRL_REG2", 1, RegisterAccess::RW, 0x00),
        RegisterDesc(0x27, "STATUS", 1, RegisterAccess::RO, 0x00),
        RegisterDesc(0x28, "PRESS_OUT_XL", 1, RegisterAccess::RO, 0x00),
        RegisterDesc(0x2B, "TEMP_OUT_L", 1, RegisterAccess::RO, 0x00),
    };
    count = sizeof(LPS25H_REGISTERS) / sizeof(RegisterDesc);
    return LPS25H_REGISTERS;
}

bool LPS25HDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    return readRegister((uint8_t)reg, buf);
}

bool LPS25HDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* LPS25HDriver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
