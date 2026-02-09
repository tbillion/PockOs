#include "sgp40_driver.h"
#include "../driver_config.h"

#if POCKETOS_SGP40_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define SGP40_CMD_MEASURE_RAW           0x260F
#define SGP40_CMD_HEATER_OFF            0x3615

SGP40Driver::SGP40Driver() : address(0), initialized(false) {}

bool SGP40Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SGP40_ENABLE_LOGGING
    Logger::info("SGP40: Initializing at address 0x" + String(address, HEX));
#endif
    
    initialized = true;
#if POCKETOS_SGP40_ENABLE_LOGGING
    Logger::info("SGP40: Initialized successfully");
#endif
    return true;
}

void SGP40Driver::deinit() {
    if (initialized) {
        sendCommand(SGP40_CMD_HEATER_OFF, nullptr, 0);
    }
    initialized = false;
}

SGP40Data SGP40Driver::readData() {
    SGP40Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t params[6] = { 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93 };
    
    if (!sendCommand(SGP40_CMD_MEASURE_RAW, params, 6)) {
        return data;
    }
    
    delay(30);
    
    uint8_t buffer[3];
    if (!readResponse(buffer, 3)) {
        return data;
    }
    
    data.voc_raw = (buffer[0] << 8) | buffer[1];
    data.voc_index = 0;
    data.valid = true;
    
    return data;
}

CapabilitySchema SGP40Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "sgp40";
    schema.tier = POCKETOS_SGP40_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("voc_raw", "VOC raw signal", "raw", "0-65535"));
    
    return schema;
}

String SGP40Driver::getParameter(const String& name) {
    return "";
}

bool SGP40Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool SGP40Driver::sendCommand(uint16_t command, const uint8_t* params, size_t paramLen) {
    Wire.beginTransmission(address);
    Wire.write((uint8_t)(command >> 8));
    Wire.write((uint8_t)(command & 0xFF));
    
    for (size_t i = 0; i < paramLen; i++) {
        Wire.write(params[i]);
    }
    
    return Wire.endTransmission() == 0;
}

bool SGP40Driver::readResponse(uint8_t* buffer, size_t length) {
    if (Wire.requestFrom(address, (uint8_t)length) != length) {
        return false;
    }
    
    for (size_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
}

#if POCKETOS_SGP40_ENABLE_REGISTER_ACCESS
static const RegisterDesc SGP40_REGISTERS[] = {
    RegisterDesc(0x260F, "MEASURE_RAW", 3, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x3615, "HEATER_OFF", 0, RegisterAccess::WO, 0x0000),
};

#define SGP40_REGISTER_COUNT (sizeof(SGP40_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* SGP40Driver::registers(size_t& count) const {
    count = SGP40_REGISTER_COUNT;
    return SGP40_REGISTERS;
}

bool SGP40Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized) {
        return false;
    }
    
    uint8_t params[6] = { 0x80, 0x00, 0xA2, 0x66, 0x66, 0x93 };
    if (!sendCommand(reg, params, 6)) {
        return false;
    }
    
    delay(30);
    return readResponse(buf, len);
}

bool SGP40Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized) {
        return false;
    }
    return sendCommand(reg, buf, len);
}

const RegisterDesc* SGP40Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < SGP40_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(SGP40_REGISTERS[i].name)) {
            return &SGP40_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
