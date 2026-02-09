#include "scd30_driver.h"
#include "../driver_config.h"
#if POCKETOS_SCD30_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define SCD30_CMD_START_CONT   0x0010
#define SCD30_CMD_STOP_CONT    0x0104
#define SCD30_CMD_SET_INTERVAL 0x4600
#define SCD30_CMD_GET_READY    0x0202
#define SCD30_CMD_READ_MEAS    0x0300
#define SCD30_CMD_SOFT_RESET   0xD304

SCD30Driver::SCD30Driver() : address(0), initialized(false) {
}

bool SCD30Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    if (!sendCommand(SCD30_CMD_SOFT_RESET)) {
#if POCKETOS_SCD30_ENABLE_LOGGING
        Logger::error("SCD30: Soft reset failed");
#endif
        return false;
    }
    delay(2000);
    
    if (!sendCommand(SCD30_CMD_SET_INTERVAL, 2)) {
#if POCKETOS_SCD30_ENABLE_LOGGING
        Logger::error("SCD30: Set interval failed");
#endif
        return false;
    }
    
    if (!sendCommand(SCD30_CMD_START_CONT, 0)) {
#if POCKETOS_SCD30_ENABLE_LOGGING
        Logger::error("SCD30: Start continuous failed");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_SCD30_ENABLE_LOGGING
    Logger::info("SCD30: Initialized");
#endif
    return true;
}

void SCD30Driver::deinit() {
    if (initialized) {
        sendCommand(SCD30_CMD_STOP_CONT);
    }
    initialized = false;
}

SCD30Data SCD30Driver::readData() {
    SCD30Data data;
    if (!initialized) return data;
    
    uint8_t buffer[18];
    Wire.beginTransmission(address);
    Wire.write(SCD30_CMD_READ_MEAS >> 8);
    Wire.write(SCD30_CMD_READ_MEAS & 0xFF);
    if (Wire.endTransmission() != 0) return data;
    
    delay(10);
    Wire.requestFrom(address, (uint8_t)18);
    size_t count = 0;
    while (Wire.available() && count < 18) {
        buffer[count++] = Wire.read();
    }
    
    if (count != 18) return data;
    
    uint32_t co2_raw = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[3] << 8) | buffer[4];
    uint32_t temp_raw = ((uint32_t)buffer[6] << 24) | ((uint32_t)buffer[7] << 16) | ((uint32_t)buffer[9] << 8) | buffer[10];
    uint32_t hum_raw = ((uint32_t)buffer[12] << 24) | ((uint32_t)buffer[13] << 16) | ((uint32_t)buffer[15] << 8) | buffer[16];
    
    float co2, temp, hum;
    memcpy(&co2, &co2_raw, sizeof(float));
    memcpy(&temp, &temp_raw, sizeof(float));
    memcpy(&hum, &hum_raw, sizeof(float));
    
    data.co2 = co2;
    data.temperature = temp;
    data.humidity = hum;
    data.valid = true;
    
    return data;
}

CapabilitySchema SCD30Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x61", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "scd30", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_SCD30_TIER_NAME, "", "", "");
    schema.addSignal("co2", ParamType::FLOAT, true, "ppm");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    schema.addCommand("read", "");
    return schema;
}

String SCD30Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "scd30";
    if (name == "tier") return POCKETOS_SCD30_TIER_NAME;
    return "";
}

bool SCD30Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool SCD30Driver::sendCommand(uint16_t cmd, uint16_t arg) {
    Wire.beginTransmission(address);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    Wire.write(arg >> 8);
    Wire.write(arg & 0xFF);
    uint8_t crc_data[2] = {(uint8_t)(arg >> 8), (uint8_t)(arg & 0xFF)};
    Wire.write(computeCRC(crc_data, 2));
    return (Wire.endTransmission() == 0);
}

bool SCD30Driver::sendCommand(uint16_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    return (Wire.endTransmission() == 0);
}

bool SCD30Driver::readData(uint8_t* buffer, size_t len) {
    Wire.requestFrom(address, (uint8_t)len);
    size_t count = 0;
    while (Wire.available() && count < len) {
        buffer[count++] = Wire.read();
    }
    return (count == len);
}

uint8_t SCD30Driver::computeCRC(uint8_t data[], uint8_t len) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

#if POCKETOS_SCD30_ENABLE_REGISTER_ACCESS
const RegisterDesc* SCD30Driver::registers(size_t& count) const {
    static const RegisterDesc SCD30_REGISTERS[] = {
        RegisterDesc(0x0010, "START_CONT", 2, RegisterAccess::WO, 0x00),
        RegisterDesc(0x0104, "STOP_CONT", 2, RegisterAccess::WO, 0x00),
        RegisterDesc(0x0300, "READ_MEAS", 2, RegisterAccess::RO, 0x00),
    };
    count = sizeof(SCD30_REGISTERS) / sizeof(RegisterDesc);
    return SCD30_REGISTERS;
}

bool SCD30Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return false;
}

bool SCD30Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return sendCommand(reg);
}

const RegisterDesc* SCD30Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
