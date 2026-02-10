#include "ms8607_driver.h"
#include "../driver_config.h"
#if POCKETOS_MS8607_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define MS8607_ADDR_PT      0x76
#define MS8607_ADDR_HUM     0x40
#define MS8607_CMD_RESET    0x1E
#define MS8607_CMD_CONV_D1  0x48
#define MS8607_CMD_CONV_D2  0x58
#define MS8607_CMD_ADC_READ 0x00
#define MS8607_CMD_PROM     0xA0
#define MS8607_HUM_HOLD     0xE5
#define MS8607_HUM_NO_HOLD  0xF5

MS8607Driver::MS8607Driver() : address(0), initialized(false) {
}

bool MS8607Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    if (!sendCommand(MS8607_ADDR_PT, MS8607_CMD_RESET)) {
#if POCKETOS_MS8607_ENABLE_LOGGING
        Logger::error("MS8607: Reset failed");
#endif
        return false;
    }
    delay(10);
    
    if (!sendCommand(MS8607_ADDR_HUM, MS8607_CMD_RESET)) {
#if POCKETOS_MS8607_ENABLE_LOGGING
        Logger::error("MS8607: Humidity reset failed");
#endif
        return false;
    }
    delay(10);
    
    initialized = true;
#if POCKETOS_MS8607_ENABLE_LOGGING
    Logger::info("MS8607: Initialized");
#endif
    return true;
}

void MS8607Driver::deinit() {
    initialized = false;
}

MS8607Data MS8607Driver::readData() {
    MS8607Data data;
    if (!initialized) return data;
    
    sendCommand(MS8607_ADDR_PT, MS8607_CMD_CONV_D2);
    delay(10);
    uint8_t temp_buf[3];
    if (readData(MS8607_ADDR_PT, temp_buf, 3)) {
        uint32_t D2 = ((uint32_t)temp_buf[0] << 16) | ((uint32_t)temp_buf[1] << 8) | temp_buf[2];
        data.temperature = (D2 / 524288.0f) - 40.0f;
    }
    
    sendCommand(MS8607_ADDR_PT, MS8607_CMD_CONV_D1);
    delay(10);
    uint8_t press_buf[3];
    if (readData(MS8607_ADDR_PT, press_buf, 3)) {
        uint32_t D1 = ((uint32_t)press_buf[0] << 16) | ((uint32_t)press_buf[1] << 8) | press_buf[2];
        data.pressure = D1 / 4096.0f;
    }
    
    sendCommand(MS8607_ADDR_HUM, MS8607_HUM_NO_HOLD);
    delay(20);
    uint8_t hum_buf[3];
    if (readData(MS8607_ADDR_HUM, hum_buf, 3)) {
        uint16_t raw_hum = ((uint16_t)hum_buf[0] << 8) | hum_buf[1];
        raw_hum &= 0xFFFC;
        data.humidity = ((125.0f * raw_hum) / 65536.0f) - 6.0f;
        if (data.humidity < 0) data.humidity = 0;
        if (data.humidity > 100) data.humidity = 100;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema MS8607Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x76", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "ms8607", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_MS8607_TIER_NAME, "", "", "");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    schema.addSignal("pressure", ParamType::FLOAT, true, "hPa");
    schema.addCommand("read", "");
    return schema;
}

String MS8607Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "ms8607";
    if (name == "tier") return POCKETOS_MS8607_TIER_NAME;
    return "";
}

bool MS8607Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool MS8607Driver::sendCommand(uint8_t addr, uint8_t cmd) {
    Wire.beginTransmission(addr);
    Wire.write(cmd);
    return (Wire.endTransmission() == 0);
}

bool MS8607Driver::readData(uint8_t addr, uint8_t* buffer, size_t len) {
    Wire.requestFrom(addr, (uint8_t)len);
    size_t count = 0;
    while (Wire.available() && count < len) {
        buffer[count++] = Wire.read();
    }
    return (count == len);
}

#if POCKETOS_MS8607_ENABLE_REGISTER_ACCESS
const RegisterDesc* MS8607Driver::registers(size_t& count) const {
    static const RegisterDesc MS8607_REGISTERS[] = {
        RegisterDesc(0x1E, "RESET", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0x48, "CONV_D1", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0x58, "CONV_D2", 1, RegisterAccess::WO, 0x00),
        RegisterDesc(0xE5, "HUM_HOLD", 1, RegisterAccess::RW, 0x00),
    };
    count = sizeof(MS8607_REGISTERS) / sizeof(RegisterDesc);
    return MS8607_REGISTERS;
}

bool MS8607Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return false;
}

bool MS8607Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return sendCommand(MS8607_ADDR_PT, (uint8_t)reg);
}

const RegisterDesc* MS8607Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
