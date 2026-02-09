#include "si7021_driver.h"
#include "../driver_config.h"

#if POCKETOS_SI7021_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// SI7021 Commands
#define SI7021_CMD_MEASURE_HUM_HOLD     0xE5  // Measure humidity, hold master mode
#define SI7021_CMD_MEASURE_HUM_NOHOLD   0xF5  // Measure humidity, no hold master mode
#define SI7021_CMD_MEASURE_TEMP_HOLD    0xE3  // Measure temperature, hold master mode
#define SI7021_CMD_MEASURE_TEMP_NOHOLD  0xF3  // Measure temperature, no hold master mode
#define SI7021_CMD_READ_TEMP_FROM_HUM   0xE0  // Read temperature from previous humidity measurement
#define SI7021_CMD_RESET                0xFE  // Reset
#define SI7021_CMD_WRITE_USER_REG       0xE6  // Write user register
#define SI7021_CMD_READ_USER_REG        0xE7  // Read user register
#define SI7021_CMD_WRITE_HEATER_REG     0x51  // Write heater control register
#define SI7021_CMD_READ_HEATER_REG      0x11  // Read heater control register

SI7021Driver::SI7021Driver() : address(0), initialized(false) {
#if POCKETOS_SI7021_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool SI7021Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SI7021_ENABLE_LOGGING
    Logger::info("SI7021: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_SI7021_ENABLE_CONFIGURATION
    // Reset sensor
    if (!sendCommand(SI7021_CMD_RESET)) {
#if POCKETOS_SI7021_ENABLE_LOGGING
        Logger::error("SI7021: Failed to send reset command");
#endif
        return false;
    }
    delay(15); // Wait for reset to complete
#endif
    
    initialized = true;
#if POCKETOS_SI7021_ENABLE_LOGGING
    Logger::info("SI7021: Initialized successfully");
#endif
    return true;
}

void SI7021Driver::deinit() {
    if (initialized) {
#if POCKETOS_SI7021_ENABLE_HEATER
        // Turn off heater before deinit
        setHeater(false);
#endif
        initialized = false;
    }
}

SI7021Data SI7021Driver::readData() {
    SI7021Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Measure humidity (no hold mode)
    if (!sendCommand(SI7021_CMD_MEASURE_HUM_NOHOLD)) {
#if POCKETOS_SI7021_ENABLE_LOGGING
        errorCount++;
        Logger::error("SI7021: Failed to send humidity measurement command");
#endif
        return data;
    }
    
    // Wait for measurement to complete
    delay(25); // Humidity measurement takes ~12ms for 12-bit resolution
    
    // Read humidity (2 bytes + CRC)
    uint8_t humBuffer[3];
    if (!readData(humBuffer, 3)) {
#if POCKETOS_SI7021_ENABLE_LOGGING
        errorCount++;
        Logger::error("SI7021: Failed to read humidity data");
#endif
        return data;
    }
    
#if POCKETOS_SI7021_ENABLE_ERROR_HANDLING
    // Verify CRC for humidity
    if (calculateCRC(humBuffer, 2) != humBuffer[2]) {
#if POCKETOS_SI7021_ENABLE_LOGGING
        errorCount++;
        Logger::error("SI7021: Humidity CRC mismatch");
#endif
        return data;
    }
#endif
    
    // Read temperature from previous humidity measurement
    if (!sendCommand(SI7021_CMD_READ_TEMP_FROM_HUM)) {
#if POCKETOS_SI7021_ENABLE_LOGGING
        errorCount++;
        Logger::error("SI7021: Failed to send temperature read command");
#endif
        return data;
    }
    
    // Read temperature (2 bytes, no CRC for this command)
    uint8_t tempBuffer[2];
    if (!readData(tempBuffer, 2)) {
#if POCKETOS_SI7021_ENABLE_LOGGING
        errorCount++;
        Logger::error("SI7021: Failed to read temperature data");
#endif
        return data;
    }
    
    // Convert raw data to physical values
    uint16_t humRaw = (humBuffer[0] << 8) | humBuffer[1];
    uint16_t tempRaw = (tempBuffer[0] << 8) | tempBuffer[1];
    
    // Humidity conversion: RH = ((125 * humRaw) / 65536) - 6
    data.humidity = ((125.0f * humRaw) / 65536.0f) - 6.0f;
    
    // Temperature conversion: T = ((175.72 * tempRaw) / 65536) - 46.85
    data.temperature = ((175.72f * tempRaw) / 65536.0f) - 46.85f;
    
    // Clamp humidity to valid range
    if (data.humidity > 100.0f) data.humidity = 100.0f;
    if (data.humidity < 0.0f) data.humidity = 0.0f;
    
    data.valid = true;
    
#if POCKETOS_SI7021_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema SI7021Driver::getSchema() const {
    CapabilitySchema schema;
    
    schema.addSetting("address", ParamType::STRING, true, "0x40", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "si7021", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_SI7021_TIER_NAME, "", "", "");
    
#if POCKETOS_SI7021_ENABLE_HEATER
    schema.addSetting("heater", ParamType::BOOL, false, "false", "", "", "");
#endif
    
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    
#if POCKETOS_SI7021_ENABLE_LOGGING
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    schema.addCommand("read", "");
    
#if POCKETOS_SI7021_ENABLE_CONFIGURATION
    schema.addCommand("reset", "");
#endif
    
    return schema;
}

String SI7021Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "si7021";
    } else if (name == "tier") {
        return POCKETOS_SI7021_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_SI7021_ENABLE_HEATER
    else if (name == "heater") {
        return getHeaterStatus() ? "true" : "false";
    }
#endif
#if POCKETOS_SI7021_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool SI7021Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_SI7021_ENABLE_HEATER
    if (name == "heater") {
        bool enable = (value == "true" || value == "1" || value == "on");
        return setHeater(enable);
    }
#endif
    return false;
}

#if POCKETOS_SI7021_ENABLE_HEATER
bool SI7021Driver::setHeater(bool enabled) {
    if (!initialized) {
        return false;
    }
    
    // Read current user register
    if (!sendCommand(SI7021_CMD_READ_USER_REG)) {
        return false;
    }
    
    uint8_t userReg;
    if (!readData(&userReg, 1)) {
        return false;
    }
    
    // Set or clear heater enable bit (bit 2)
    if (enabled) {
        userReg |= 0x04;
    } else {
        userReg &= ~0x04;
    }
    
    // Write back user register
    Wire.beginTransmission(address);
    Wire.write(SI7021_CMD_WRITE_USER_REG);
    Wire.write(userReg);
    return (Wire.endTransmission() == 0);
}

bool SI7021Driver::getHeaterStatus() {
    if (!initialized) {
        return false;
    }
    
    // Read user register
    if (!sendCommand(SI7021_CMD_READ_USER_REG)) {
        return false;
    }
    
    uint8_t userReg;
    if (!readData(&userReg, 1)) {
        return false;
    }
    
    // Check heater enable bit (bit 2)
    return (userReg & 0x04) != 0;
}
#endif

bool SI7021Driver::sendCommand(uint8_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(cmd);
    return (Wire.endTransmission() == 0);
}

bool SI7021Driver::readData(uint8_t* buffer, size_t len) {
    Wire.requestFrom(address, (uint8_t)len);
    
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < len) {
        buffer[bytesRead++] = Wire.read();
    }
    
    return (bytesRead == len);
}

uint8_t SI7021Driver::calculateCRC(const uint8_t* data, size_t len) {
    // CRC-8 with polynomial 0x31 (x^8 + x^5 + x^4 + 1)
    uint8_t crc = 0x00;
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; bit--) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    
    return crc;
}

} // namespace PocketOS
