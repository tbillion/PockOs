#include "aht10_driver.h"
#include "../driver_config.h"

#if POCKETOS_AHT10_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// AHT10 Commands
#define AHT10_CMD_INIT          0xE1  // Initialization command
#define AHT10_CMD_TRIGGER       0xAC  // Trigger measurement
#define AHT10_CMD_SOFT_RESET    0xBA  // Soft reset

AHT10Driver::AHT10Driver() : address(0), initialized(false) {
#if POCKETOS_AHT10_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool AHT10Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AHT10_ENABLE_LOGGING
    Logger::info("AHT10: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_AHT10_ENABLE_CONFIGURATION
    // Soft reset
    if (!sendCommand(AHT10_CMD_SOFT_RESET, 0x00, 0x00)) {
#if POCKETOS_AHT10_ENABLE_LOGGING
        Logger::error("AHT10: Failed to send reset command");
#endif
        return false;
    }
    delay(20); // Wait for reset to complete
    
    // Initialize sensor
    if (!sendCommand(AHT10_CMD_INIT, 0x08, 0x00)) {
#if POCKETOS_AHT10_ENABLE_LOGGING
        Logger::error("AHT10: Failed to initialize");
#endif
        return false;
    }
    delay(10); // Wait for initialization
#else
    delay(40); // Wait for sensor to be ready
#endif
    
    initialized = true;
#if POCKETOS_AHT10_ENABLE_LOGGING
    Logger::info("AHT10: Initialized successfully");
#endif
    return true;
}

void AHT10Driver::deinit() {
    initialized = false;
}

AHT10Data AHT10Driver::readData() {
    AHT10Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Trigger measurement
    if (!sendCommand(AHT10_CMD_TRIGGER, 0x33, 0x00)) {
#if POCKETOS_AHT10_ENABLE_LOGGING
        errorCount++;
        Logger::error("AHT10: Failed to trigger measurement");
#endif
        return data;
    }
    
    // Wait for measurement to complete
    delay(80); // AHT10 measurement takes ~75ms
    
    // Read 6 bytes: status, humidity[19:12], humidity[11:4], humidity[3:0]+temp[19:16], temp[15:8], temp[7:0]
    uint8_t buffer[6];
    if (!readData(buffer, 6)) {
#if POCKETOS_AHT10_ENABLE_LOGGING
        errorCount++;
        Logger::error("AHT10: Failed to read measurement data");
#endif
        return data;
    }
    
    // Check if measurement is ready (busy bit should be 0)
    if (buffer[0] & 0x80) {
#if POCKETOS_AHT10_ENABLE_LOGGING
        errorCount++;
        Logger::error("AHT10: Sensor busy");
#endif
        return data;
    }
    
#if POCKETOS_AHT10_ENABLE_ERROR_HANDLING
    // Verify CRC (byte 6 if available, but AHT10 doesn't always provide it)
    // AHT10 doesn't have built-in CRC, skip validation
#endif
    
    // Extract raw data
    uint32_t humRaw = ((uint32_t)buffer[1] << 12) | ((uint32_t)buffer[2] << 4) | ((uint32_t)buffer[3] >> 4);
    uint32_t tempRaw = (((uint32_t)buffer[3] & 0x0F) << 16) | ((uint32_t)buffer[4] << 8) | (uint32_t)buffer[5];
    
    // Convert to physical values
    // Humidity: RH = (rawValue / 1048576) * 100
    data.humidity = (humRaw / 1048576.0f) * 100.0f;
    
    // Temperature: T = (rawValue / 1048576) * 200 - 50
    data.temperature = (tempRaw / 1048576.0f) * 200.0f - 50.0f;
    
    // Clamp humidity to valid range
    if (data.humidity > 100.0f) data.humidity = 100.0f;
    if (data.humidity < 0.0f) data.humidity = 0.0f;
    
    data.valid = true;
    
#if POCKETOS_AHT10_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema AHT10Driver::getSchema() const {
    CapabilitySchema schema;
    
    schema.addSetting("address", ParamType::STRING, true, "0x38", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "aht10", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_AHT10_TIER_NAME, "", "", "");
    
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    
#if POCKETOS_AHT10_ENABLE_LOGGING
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    schema.addCommand("read", "");
    
#if POCKETOS_AHT10_ENABLE_CONFIGURATION
    schema.addCommand("reset", "");
#endif
    
    return schema;
}

String AHT10Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "aht10";
    } else if (name == "tier") {
        return POCKETOS_AHT10_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_AHT10_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool AHT10Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool AHT10Driver::sendCommand(uint8_t cmd, uint8_t param1, uint8_t param2) {
    Wire.beginTransmission(address);
    Wire.write(cmd);
    Wire.write(param1);
    Wire.write(param2);
    return (Wire.endTransmission() == 0);
}

bool AHT10Driver::readData(uint8_t* buffer, size_t len) {
    Wire.requestFrom(address, (uint8_t)len);
    
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < len) {
        buffer[bytesRead++] = Wire.read();
    }
    
    return (bytesRead == len);
}

uint8_t AHT10Driver::calculateCRC(const uint8_t* data, size_t len) {
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
