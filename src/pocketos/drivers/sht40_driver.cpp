#include "sht40_driver.h"
#include "../driver_config.h"

#if POCKETOS_SHT40_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// SHT40 Commands
#define SHT40_CMD_MEASURE_HIGH_PREC   0xFD  // High precision measurement
#define SHT40_CMD_MEASURE_MED_PREC    0xF6  // Medium precision measurement
#define SHT40_CMD_MEASURE_LOW_PREC    0xE0  // Low precision measurement
#define SHT40_CMD_SOFT_RESET          0x94  // Soft reset
#define SHT40_CMD_READ_SERIAL         0x89  // Read serial number

SHT40Driver::SHT40Driver() : address(0), initialized(false) {
#if POCKETOS_SHT40_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool SHT40Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SHT40_ENABLE_LOGGING
    Logger::info("SHT40: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_SHT40_ENABLE_CONFIGURATION
    // Soft reset
    if (!sendCommand(SHT40_CMD_SOFT_RESET)) {
#if POCKETOS_SHT40_ENABLE_LOGGING
        Logger::error("SHT40: Failed to send reset command");
#endif
        return false;
    }
    delay(1); // Wait for reset to complete
#endif
    
    initialized = true;
#if POCKETOS_SHT40_ENABLE_LOGGING
    Logger::info("SHT40: Initialized successfully");
#endif
    return true;
}

void SHT40Driver::deinit() {
    initialized = false;
}

SHT40Data SHT40Driver::readData() {
    SHT40Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Send measurement command (high precision)
    if (!sendCommand(SHT40_CMD_MEASURE_HIGH_PREC)) {
#if POCKETOS_SHT40_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT40: Failed to send measurement command");
#endif
        return data;
    }
    
    // Wait for measurement to complete
    delay(10); // High precision measurement takes ~8.3ms
    
    // Read 6 bytes: temp MSB, temp LSB, temp CRC, hum MSB, hum LSB, hum CRC
    uint8_t buffer[6];
    if (!readData(buffer, 6)) {
#if POCKETOS_SHT40_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT40: Failed to read measurement data");
#endif
        return data;
    }
    
#if POCKETOS_SHT40_ENABLE_ERROR_HANDLING
    // Verify CRC for temperature
    if (calculateCRC(buffer, 2) != buffer[2]) {
#if POCKETOS_SHT40_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT40: Temperature CRC mismatch");
#endif
        return data;
    }
    
    // Verify CRC for humidity
    if (calculateCRC(buffer + 3, 2) != buffer[5]) {
#if POCKETOS_SHT40_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHT40: Humidity CRC mismatch");
#endif
        return data;
    }
#endif
    
    // Convert raw data to temperature and humidity
    uint16_t tempRaw = (buffer[0] << 8) | buffer[1];
    uint16_t humRaw = (buffer[3] << 8) | buffer[4];
    
    // Temperature conversion: T = -45 + 175 * (rawValue / 65535)
    data.temperature = -45.0f + 175.0f * (tempRaw / 65535.0f);
    
    // Humidity conversion: RH = -6 + 125 * (rawValue / 65535)
    data.humidity = -6.0f + 125.0f * (humRaw / 65535.0f);
    
    // Clamp humidity to valid range
    if (data.humidity > 100.0f) data.humidity = 100.0f;
    if (data.humidity < 0.0f) data.humidity = 0.0f;
    
    data.valid = true;
    
#if POCKETOS_SHT40_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema SHT40Driver::getSchema() const {
    CapabilitySchema schema;
    
    schema.addSetting("address", ParamType::STRING, true, "0x44", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "sht40", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_SHT40_TIER_NAME, "", "", "");
    
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    
#if POCKETOS_SHT40_ENABLE_LOGGING
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    schema.addCommand("read", "");
    
#if POCKETOS_SHT40_ENABLE_CONFIGURATION
    schema.addCommand("reset", "");
#endif
    
    return schema;
}

String SHT40Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "sht40";
    } else if (name == "tier") {
        return POCKETOS_SHT40_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_SHT40_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool SHT40Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool SHT40Driver::sendCommand(uint8_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(cmd);
    return (Wire.endTransmission() == 0);
}

bool SHT40Driver::readData(uint8_t* buffer, size_t len) {
    Wire.requestFrom(address, (uint8_t)len);
    
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < len) {
        buffer[bytesRead++] = Wire.read();
    }
    
    return (bytesRead == len);
}

uint8_t SHT40Driver::calculateCRC(const uint8_t* data, size_t len) {
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
