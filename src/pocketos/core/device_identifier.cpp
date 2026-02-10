#include "device_identifier.h"
#include "hal.h"
#include "logger.h"
#include <Wire.h>
#include "../drivers/mcp2515_driver.h"
#include "../drivers/nrf24l01_driver.h"
#include "../drivers/w5500_driver.h"
#include "../drivers/ili9341_driver.h"
#include "../drivers/st7789_driver.h"

namespace PocketOS {

void DeviceIdentifier::init() {
    Logger::info("DeviceIdentifier initialized");
}

DeviceIdentification DeviceIdentifier::identifyEndpoint(const String& endpoint) {
    // Parse endpoint to determine type and address
    if (endpoint.startsWith("i2c0:0x") || endpoint.startsWith("i2c0:")) {
        // Extract I2C address
        int colonPos = endpoint.indexOf(':');
        if (colonPos > 0) {
            String addrStr = endpoint.substring(colonPos + 1);
            uint8_t address;
            if (addrStr.startsWith("0x") || addrStr.startsWith("0X")) {
                address = (uint8_t)strtol(addrStr.c_str(), nullptr, 16);
            } else {
                address = (uint8_t)addrStr.toInt();
            }
            return identifyI2C(address);
        }
    }

    if (endpoint.startsWith("spi")) {
        return identifySPI(endpoint);
    }
    
    DeviceIdentification result;
    result.deviceClass = "unknown";
    result.confidence = "unknown";
    result.details = "Endpoint type not supported for identification";
    result.identified = false;
    return result;
}

DeviceIdentification DeviceIdentifier::identifyI2C(uint8_t address) {
    Logger::info("Identifying I2C device at address 0x" + String(address, HEX));
    
    // Try BME280 first
    DeviceIdentification result = identifyBME280(address);
    if (result.identified) {
        return result;
    }
    
    // Future: Add more device identifications here
    // result = identifyBME680(address);
    // result = identifySHT31(address);
    // etc.
    
    // Device responded but not identified
    result.deviceClass = "unknown";
    result.confidence = "low";
    result.details = "Device present but not in identification database";
    result.identified = false;
    return result;
}

DeviceIdentification DeviceIdentifier::identifySPI(const String& endpoint) {
    Logger::info("Identifying SPI device at " + endpoint);

    // Attempt known SPI probes in priority order
    if (MCP2515Driver::identifyProbe(endpoint)) {
        DeviceIdentification id;
        id.deviceClass = "mcp2515";
        id.confidence = "high";
        id.details = "CANSTAT/CANCTRL probe succeeded";
        id.identified = true;
        return id;
    }

    if (NRF24L01Driver::identifyProbe(endpoint)) {
        DeviceIdentification id;
        id.deviceClass = "nrf24l01+";
        id.confidence = "medium";
        id.details = "STATUS/CONFIG probe succeeded";
        id.identified = true;
        return id;
    }

    if (W5500Driver::identifyProbe(endpoint)) {
        DeviceIdentification id;
        id.deviceClass = "w5500";
        id.confidence = "high";
        id.details = "VERSIONR probe succeeded";
        id.identified = true;
        return id;
    }

    if (ILI9341Driver::identifyProbe(endpoint)) {
        DeviceIdentification id;
        id.deviceClass = "ili9341";
        id.confidence = "medium";
        id.details = "RDMODE/RDDID probe succeeded";
        id.identified = true;
        return id;
    }

    if (ST7789Driver::identifyProbe(endpoint)) {
        DeviceIdentification id;
        id.deviceClass = "st7789";
        id.confidence = "medium";
        id.details = "Read display ID probe succeeded";
        id.identified = true;
        return id;
    }

    DeviceIdentification result;
    result.deviceClass = "unknown";
    result.confidence = "low";
    result.details = "SPI probes did not match known devices";
    result.identified = false;
    return result;
}

DeviceIdentification DeviceIdentifier::identifyBME280(uint8_t address) {
    DeviceIdentification result;
    
    // BME280 has chip ID 0x60 at register 0xD0
    // Valid addresses: 0x76 or 0x77
    if (address != 0x76 && address != 0x77) {
        result.identified = false;
        return result;
    }
    
    uint8_t chipId = 0;
    if (!readI2CRegister(address, 0xD0, &chipId)) {
        result.identified = false;
        result.details = "Failed to read chip ID register";
        return result;
    }
    
    if (chipId == 0x60) {
        result.deviceClass = "bme280";
        result.confidence = "high";
        result.details = "Chip ID: 0x60, Address: 0x" + String(address, HEX);
        result.identified = true;
        Logger::info("BME280 identified at 0x" + String(address, HEX));
    } else {
        result.identified = false;
        result.details = "Chip ID mismatch: expected 0x60, got 0x" + String(chipId, HEX);
    }
    
    return result;
}

bool DeviceIdentifier::readI2CRegister(uint8_t address, uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        *value = Wire.read();
        return true;
    }
    
    return false;
}

bool DeviceIdentifier::readI2CRegisters(uint8_t address, uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)len);
    size_t count = 0;
    while (Wire.available() && count < len) {
        buffer[count++] = Wire.read();
    }
    
    return (count == len);
}

} // namespace PocketOS
