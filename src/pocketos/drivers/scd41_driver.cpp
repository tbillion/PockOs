#include "scd41_driver.h"
#include "../driver_config.h"
#if POCKETOS_SCD41_ENABLE_LOGGING
#include "../core/logger.h"
#endif
#include <Wire.h>

namespace PocketOS {

#define SCD41_CMD_START_PERIODIC    0x21b1
#define SCD41_CMD_STOP_PERIODIC     0x3f86
#define SCD41_CMD_READ_MEAS         0xec05
#define SCD41_CMD_GET_READY         0xe4b8
#define SCD41_CMD_REINIT            0x3646

SCD41Driver::SCD41Driver() : address(0), initialized(false) {
}

bool SCD41Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
    if (!sendCommand(SCD41_CMD_STOP_PERIODIC)) {
#if POCKETOS_SCD41_ENABLE_LOGGING
        Logger::error("SCD41: Stop periodic failed");
#endif
    }
    delay(500);
    
    if (!sendCommand(SCD41_CMD_REINIT)) {
#if POCKETOS_SCD41_ENABLE_LOGGING
        Logger::error("SCD41: Reinit failed");
#endif
        return false;
    }
    delay(20);
    
    if (!sendCommand(SCD41_CMD_START_PERIODIC)) {
#if POCKETOS_SCD41_ENABLE_LOGGING
        Logger::error("SCD41: Start periodic failed");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_SCD41_ENABLE_LOGGING
    Logger::info("SCD41: Initialized");
#endif
    return true;
}

void SCD41Driver::deinit() {
    if (initialized) {
        sendCommand(SCD41_CMD_STOP_PERIODIC);
    }
    initialized = false;
}

SCD41Data SCD41Driver::readData() {
    SCD41Data data;
    if (!initialized) return data;
    
    uint8_t buffer[9];
    Wire.beginTransmission(address);
    Wire.write(SCD41_CMD_READ_MEAS >> 8);
    Wire.write(SCD41_CMD_READ_MEAS & 0xFF);
    if (Wire.endTransmission() != 0) return data;
    
    delay(10);
    Wire.requestFrom(address, (uint8_t)9);
    size_t count = 0;
    while (Wire.available() && count < 9) {
        buffer[count++] = Wire.read();
    }
    
    if (count != 9) return data;
    
    uint16_t co2_raw = ((uint16_t)buffer[0] << 8) | buffer[1];
    uint16_t temp_raw = ((uint16_t)buffer[3] << 8) | buffer[4];
    uint16_t hum_raw = ((uint16_t)buffer[6] << 8) | buffer[7];
    
    data.co2 = co2_raw;
    data.temperature = -45.0f + 175.0f * (temp_raw / 65536.0f);
    data.humidity = 100.0f * (hum_raw / 65536.0f);
    data.valid = true;
    
    return data;
}

CapabilitySchema SCD41Driver::getSchema() const {
    CapabilitySchema schema;
    schema.addSetting("address", ParamType::STRING, true, "0x62", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "scd41", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_SCD41_TIER_NAME, "", "", "");
    schema.addSignal("co2", ParamType::FLOAT, true, "ppm");
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    schema.addCommand("read", "");
    return schema;
}

String SCD41Driver::getParameter(const String& name) {
    if (name == "address") return "0x" + String(address, HEX);
    if (name == "driver") return "scd41";
    if (name == "tier") return POCKETOS_SCD41_TIER_NAME;
    return "";
}

bool SCD41Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool SCD41Driver::sendCommand(uint16_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    return (Wire.endTransmission() == 0);
}

bool SCD41Driver::readData(uint8_t* buffer, size_t len) {
    Wire.requestFrom(address, (uint8_t)len);
    size_t count = 0;
    while (Wire.available() && count < len) {
        buffer[count++] = Wire.read();
    }
    return (count == len);
}

uint8_t SCD41Driver::computeCRC(uint8_t data[], uint8_t len) {
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

#if POCKETOS_SCD41_ENABLE_REGISTER_ACCESS
const RegisterDesc* SCD41Driver::registers(size_t& count) const {
    static const RegisterDesc SCD41_REGISTERS[] = {
        RegisterDesc(0x21b1, "START_PERIODIC", 2, RegisterAccess::WO, 0x00),
        RegisterDesc(0x3f86, "STOP_PERIODIC", 2, RegisterAccess::WO, 0x00),
        RegisterDesc(0xec05, "READ_MEAS", 2, RegisterAccess::RO, 0x00),
    };
    count = sizeof(SCD41_REGISTERS) / sizeof(RegisterDesc);
    return SCD41_REGISTERS;
}

bool SCD41Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return false;
}

bool SCD41Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized) return false;
    return sendCommand(reg);
}

const RegisterDesc* SCD41Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

} // namespace PocketOS
