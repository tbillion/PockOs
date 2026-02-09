#include "veml6070_driver.h"
#include "../driver_config.h"

#if POCKETOS_VEML6070_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define VEML6070_ADDR_CMD  0x38
#define VEML6070_ADDR_LSB  0x38
#define VEML6070_ADDR_MSB  0x39

VEML6070Driver::VEML6070Driver() : address(0), initialized(false), integrationTime(0x02) {}

bool VEML6070Driver::init(uint8_t i2cAddress) {
    address = VEML6070_ADDR_CMD;
    
#if POCKETOS_VEML6070_ENABLE_LOGGING
    Logger::info("VEML6070: Initializing");
#endif
    
    if (!writeCommand(integrationTime)) {
#if POCKETOS_VEML6070_ENABLE_LOGGING
        Logger::error("VEML6070: Failed to initialize");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_VEML6070_ENABLE_LOGGING
    Logger::info("VEML6070: Initialized successfully");
#endif
    return true;
}

void VEML6070Driver::deinit() {
    initialized = false;
}

VEML6070Data VEML6070Driver::readData() {
    VEML6070Data data;
    if (!initialized) return data;
    
    delay(100);
    
    uint16_t uv;
    if (!readData(&uv)) return data;
    
    data.uv = uv;
    data.uvIndex = calculateUVIndex(uv);
    data.valid = true;
    
    return data;
}

float VEML6070Driver::calculateUVIndex(uint16_t uv) {
    return uv / 227.0;
}

CapabilitySchema VEML6070Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "veml6070";
    schema.tier = POCKETOS_VEML6070_TIER_NAME;
    schema.category = "uv";
    schema.addOutput("uv", "uint16", "UV reading");
    schema.addOutput("uvIndex", "float", "UV index");
    return schema;
}

String VEML6070Driver::getParameter(const String& name) { return ""; }
bool VEML6070Driver::setParameter(const String& name, const String& value) { return false; }

bool VEML6070Driver::writeCommand(uint8_t cmd) {
    Wire.beginTransmission(VEML6070_ADDR_CMD);
    Wire.write(cmd);
    return Wire.endTransmission() == 0;
}

bool VEML6070Driver::readData(uint16_t* value) {
    Wire.requestFrom(VEML6070_ADDR_MSB, (uint8_t)1);
    if (Wire.available() != 1) return false;
    uint8_t msb = Wire.read();
    
    Wire.requestFrom(VEML6070_ADDR_LSB, (uint8_t)1);
    if (Wire.available() != 1) return false;
    uint8_t lsb = Wire.read();
    
    *value = (msb << 8) | lsb;
    return true;
}

#if POCKETOS_VEML6070_ENABLE_REGISTER_ACCESS
const RegisterDesc* VEML6070Driver::registers(size_t& count) const {
    count = 0;
    return nullptr;
}

bool VEML6070Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) { return false; }
bool VEML6070Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) { return false; }
const RegisterDesc* VEML6070Driver::findRegisterByName(const String& name) const { return nullptr; }
#endif

} // namespace PocketOS
