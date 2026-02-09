#include "sht31_driver.h"
#include "../driver_config.h"

#if POCKETOS_SHT31_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// SHT31 Commands
#define SHT31_CMD_MEASURE_HIGH_REP   0x2400  // High repeatability measurement
#define SHT31_CMD_MEASURE_MED_REP    0x240B  // Medium repeatability measurement
#define SHT31_CMD_MEASURE_LOW_REP    0x2416  // Low repeatability measurement
#define SHT31_CMD_READ_STATUS        0xF32D  // Read status register
#define SHT31_CMD_CLEAR_STATUS       0x3041  // Clear status register
#define SHT31_CMD_SOFT_RESET         0x30A2  // Soft reset
#define SHT31_CMD_HEATER_ENABLE      0x306D  // Enable heater
#define SHT31_CMD_HEATER_DISABLE     0x3066  // Disable heater

SHT31Driver::SHT31Driver() : address(0), initialized(false) {
#if POCKETOS_SHT31_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool SHT31Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SHT31_ENABLE_LOGGING
    Logger::info("SHT31: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_SHT31_ENABLE_CONFIGURATION
    // Soft reset
    if (!sendCommand(SHT31_CMD_SOFT_RESET)) {
#if POCKETOS_SHT31_ENABLE_LOGGING
        Logger::error("SHT31: Failed to send reset command");
#endif
        return false;
    }
    delay(15); // Wait for reset to complete
    
    // Clear status register
    if (!sendCommand(SHT31_CMD_CLEAR_STATUS)) {
#if POCKETOS_SHT31_ENABLE_LOGGING
        Logger::error("SHT31: Failed to clear status");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_SHT31_ENABLE_LOGGING
    Logger::info("SHT31: Initialized successfully");
#endif
    return true;
}

void SHT31Driver::deinit() {
    if (initialized) {
#if POCKETOS_SHT31_ENABLE_HEATER
        // Turn off heater before deinit
        setHeater(false);
#endif
        initialized = false;
    }
}

SHT31Data SHT31Driver::readData() {
    SHT31Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Send measurement command (high repeatability)
    if (!sendCommand(SHT31_CMD_MEASURE_HIGH_REP)) {
#if POCKETOS_SHT31_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT31: Failed to send measurement command");
#endif
        return data;
    }
    
    // Wait for measurement to complete
    delay(16); // High repeatability measurement takes ~15.5ms
    
    // Read 6 bytes: temp MSB, temp LSB, temp CRC, hum MSB, hum LSB, hum CRC
    uint8_t buffer[6];
    if (!readData(buffer, 6)) {
#if POCKETOS_SHT31_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT31: Failed to read measurement data");
#endif
        return data;
    }
    
#if POCKETOS_SHT31_ENABLE_ERROR_HANDLING
    // Verify CRC for temperature
    if (calculateCRC(buffer, 2) != buffer[2]) {
#if POCKETOS_SHT31_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT31: Temperature CRC mismatch");
#endif
        return data;
    }
    
    // Verify CRC for humidity
    if (calculateCRC(buffer + 3, 2) != buffer[5]) {
#if POCKETOS_SHT31_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT31: Humidity CRC mismatch");
#endif
        return data;
    }
#endif
    
    // Convert raw data to temperature and humidity
    uint16_t tempRaw = (buffer[0] << 8) | buffer[1];
    uint16_t humRaw = (buffer[3] << 8) | buffer[4];
    
    // Temperature conversion: T = -45 + 175 * (rawValue / 65535)
    data.temperature = -45.0f + 175.0f * (tempRaw / 65535.0f);
    
    // Humidity conversion: RH = 100 * (rawValue / 65535)
    data.humidity = 100.0f * (humRaw / 65535.0f);
    
    // Clamp humidity to valid range
    if (data.humidity > 100.0f) data.humidity = 100.0f;
    if (data.humidity < 0.0f) data.humidity = 0.0f;
    
    data.valid = true;
    
#if POCKETOS_SHT31_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema SHT31Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x44", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "sht31", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_SHT31_TIER_NAME, "", "", "");
    
#if POCKETOS_SHT31_ENABLE_HEATER
    // Heater control (Tier 1 only)
    schema.addSetting("heater", ParamType::BOOL, false, "false", "", "", "");
#endif
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    
#if POCKETOS_SHT31_ENABLE_LOGGING
    // Diagnostic signals (Tier 1 only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
#if POCKETOS_SHT31_ENABLE_CONFIGURATION
    schema.addCommand("reset", "");
#endif
    
    return schema;
}

String SHT31Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "sht31";
    } else if (name == "tier") {
        return POCKETOS_SHT31_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_SHT31_ENABLE_HEATER
    else if (name == "heater") {
        return getHeaterStatus() ? "true" : "false";
    }
#endif
#if POCKETOS_SHT31_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool SHT31Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_SHT31_ENABLE_HEATER
    if (name == "heater") {
        bool enable = (value == "true" || value == "1" || value == "on");
        return setHeater(enable);
    }
#endif
    return false;
}

#if POCKETOS_SHT31_ENABLE_HEATER
bool SHT31Driver::setHeater(bool enabled) {
    if (!initialized) {
        return false;
    }
    
    uint16_t cmd = enabled ? SHT31_CMD_HEATER_ENABLE : SHT31_CMD_HEATER_DISABLE;
    return sendCommand(cmd);
}

bool SHT31Driver::getHeaterStatus() {
    if (!initialized) {
        return false;
    }
    
    // Send status read command
    if (!sendCommand(SHT31_CMD_READ_STATUS)) {
        return false;
    }
    
    // Read 3 bytes: status MSB, status LSB, CRC
    uint8_t buffer[3];
    if (!readData(buffer, 3)) {
        return false;
    }
    
    // Heater bit is bit 13 in the status register
    uint16_t status = (buffer[0] << 8) | buffer[1];
    return (status & 0x2000) != 0;
}
#endif

bool SHT31Driver::sendCommand(uint16_t cmd) {
    Wire.beginTransmission(address);
    Wire.write((uint8_t)(cmd >> 8));   // MSB
    Wire.write((uint8_t)(cmd & 0xFF)); // LSB
    return (Wire.endTransmission() == 0);
}

bool SHT31Driver::readData(uint8_t* buffer, size_t len) {
    Wire.requestFrom(address, (uint8_t)len);
    
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < len) {
        buffer[bytesRead++] = Wire.read();
    }
    
    return (bytesRead == len);
}

uint8_t SHT31Driver::calculateCRC(const uint8_t* data, size_t len) {
    // CRC-8 with polynomial 0x31 (x^8 + x^5 + x^4 + 1)
    uint8_t crc = 0xFF;
    
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
