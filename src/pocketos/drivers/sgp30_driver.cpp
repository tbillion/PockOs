#include "sgp30_driver.h"
#include "../driver_config.h"

#if POCKETOS_SGP30_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define SGP30_CMD_INIT_AIR_QUALITY      0x2003
#define SGP30_CMD_MEASURE_AIR_QUALITY   0x2008
#define SGP30_CMD_GET_FEATURE_SET       0x202F
#define SGP30_CMD_GET_SERIAL_ID         0x3682

SGP30Driver::SGP30Driver() : address(0), initialized(false) {}

bool SGP30Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SGP30_ENABLE_LOGGING
    Logger::info("SGP30: Initializing at address 0x" + String(address, HEX));
#endif
    
    if (!sendCommand(SGP30_CMD_INIT_AIR_QUALITY)) {
#if POCKETOS_SGP30_ENABLE_LOGGING
        Logger::error("SGP30: Failed to initialize air quality measurement");
#endif
        return false;
    }
    
    delay(10);
    
    initialized = true;
#if POCKETOS_SGP30_ENABLE_LOGGING
    Logger::info("SGP30: Initialized successfully");
#endif
    return true;
}

void SGP30Driver::deinit() {
    initialized = false;
}

SGP30Data SGP30Driver::readData() {
    SGP30Data data;
    
    if (!initialized) {
        return data;
    }
    
    if (!sendCommand(SGP30_CMD_MEASURE_AIR_QUALITY)) {
        return data;
    }
    
    delay(12);
    
    uint8_t buffer[6];
    if (!readResponse(buffer, 6)) {
        return data;
    }
    
    data.eco2 = (buffer[0] << 8) | buffer[1];
    data.tvoc = (buffer[3] << 8) | buffer[4];
    data.valid = true;
    
    return data;
}

CapabilitySchema SGP30Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "sgp30";
    schema.tier = POCKETOS_SGP30_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("eco2", "Equivalent CO2", "ppm", "400-60000"));
    schema.outputs.push_back(OutputDesc("tvoc", "Total VOC", "ppb", "0-60000"));
    
    return schema;
}

String SGP30Driver::getParameter(const String& name) {
    return "";
}

bool SGP30Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool SGP30Driver::sendCommand(uint16_t command) {
    Wire.beginTransmission(address);
    Wire.write((uint8_t)(command >> 8));
    Wire.write((uint8_t)(command & 0xFF));
    return Wire.endTransmission() == 0;
}

bool SGP30Driver::readResponse(uint8_t* buffer, size_t length) {
    if (Wire.requestFrom(address, (uint8_t)length) != length) {
        return false;
    }
    
    for (size_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
}

#if POCKETOS_SGP30_ENABLE_REGISTER_ACCESS
static const RegisterDesc SGP30_REGISTERS[] = {
    RegisterDesc(0x2003, "INIT_AIR_QUALITY", 0, RegisterAccess::WO, 0x0000),
    RegisterDesc(0x2008, "MEASURE_AIR_QUALITY", 6, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x202F, "GET_FEATURE_SET", 3, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x3682, "GET_SERIAL_ID", 9, RegisterAccess::RO, 0x0000),
};

#define SGP30_REGISTER_COUNT (sizeof(SGP30_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* SGP30Driver::registers(size_t& count) const {
    count = SGP30_REGISTER_COUNT;
    return SGP30_REGISTERS;
}

bool SGP30Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized) {
        return false;
    }
    
    if (!sendCommand(reg)) {
        return false;
    }
    
    delay(12);
    return readResponse(buf, len);
}

bool SGP30Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized) {
        return false;
    }
    return sendCommand(reg);
}

const RegisterDesc* SGP30Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < SGP30_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(SGP30_REGISTERS[i].name)) {
            return &SGP30_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
