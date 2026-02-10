#include "ens160_driver.h"
#include "../driver_config.h"

#if POCKETOS_ENS160_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// ENS160 Register addresses
#define ENS160_REG_PART_ID        0x00
#define ENS160_REG_OPMODE         0x10
#define ENS160_REG_CONFIG         0x11
#define ENS160_REG_COMMAND        0x12
#define ENS160_REG_DATA_STATUS    0x20
#define ENS160_REG_DATA_AQI       0x21
#define ENS160_REG_DATA_TVOC      0x22
#define ENS160_REG_DATA_ECO2      0x24
#define ENS160_REG_TEMP_IN        0x13
#define ENS160_REG_RH_IN          0x15

#define ENS160_PART_ID            0x0160

ENS160Driver::ENS160Driver() : address(0), initialized(false) {}

bool ENS160Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_ENS160_ENABLE_LOGGING
    Logger::info("ENS160: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t partId[2];
    if (!readRegister(ENS160_REG_PART_ID, partId, 2)) {
#if POCKETOS_ENS160_ENABLE_LOGGING
        Logger::error("ENS160: Failed to read part ID");
#endif
        return false;
    }
    
    uint16_t id = partId[0] | (partId[1] << 8);
    if (id != ENS160_PART_ID) {
#if POCKETOS_ENS160_ENABLE_LOGGING
        Logger::error("ENS160: Invalid part ID: 0x" + String(id, HEX));
#endif
        return false;
    }
    
    // Reset sensor
    writeRegister(ENS160_REG_OPMODE, 0x00);
    delay(10);
    
#if POCKETOS_ENS160_ENABLE_CONFIGURATION
    // Clear GPR registers
    writeRegister(ENS160_REG_COMMAND, 0xCC);
    delay(10);
#endif
    
    // Set standard operating mode
    writeRegister(ENS160_REG_OPMODE, 0x02);
    delay(20);
    
    initialized = true;
#if POCKETOS_ENS160_ENABLE_LOGGING
    Logger::info("ENS160: Initialized successfully");
#endif
    return true;
}

void ENS160Driver::deinit() {
    if (initialized) {
        writeRegister(ENS160_REG_OPMODE, 0x00);
    }
    initialized = false;
}

ENS160Data ENS160Driver::readData() {
    ENS160Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t status = 0;
    if (!readRegister(ENS160_REG_DATA_STATUS, &status, 1)) {
        return data;
    }
    
    if (!(status & 0x02)) {
        return data;
    }
    
    uint8_t aqi = 0;
    if (!readRegister(ENS160_REG_DATA_AQI, &aqi, 1)) {
        return data;
    }
    
    uint8_t tvoc_buf[2];
    if (!readRegister(ENS160_REG_DATA_TVOC, tvoc_buf, 2)) {
        return data;
    }
    
    uint8_t eco2_buf[2];
    if (!readRegister(ENS160_REG_DATA_ECO2, eco2_buf, 2)) {
        return data;
    }
    
    data.aqi = static_cast<AQI>(aqi);
    data.tvoc = tvoc_buf[0] | (tvoc_buf[1] << 8);
    data.eco2 = eco2_buf[0] | (eco2_buf[1] << 8);
    data.valid = true;
    
    return data;
}

CapabilitySchema ENS160Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ens160";
    schema.tier = POCKETOS_ENS160_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("tvoc", "Total VOC", "ppb", "0-65000"));
    schema.outputs.push_back(OutputDesc("eco2", "Equivalent CO2", "ppm", "400-65000"));
    schema.outputs.push_back(OutputDesc("aqi", "Air quality index", "enum", "1-5"));
    
    return schema;
}

String ENS160Driver::getParameter(const String& name) {
    return "";
}

bool ENS160Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool ENS160Driver::readRegister(uint8_t reg, uint8_t* buffer, size_t length) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)length) != length) {
        return false;
    }
    
    for (size_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    
    return true;
}

bool ENS160Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

#if POCKETOS_ENS160_ENABLE_REGISTER_ACCESS
static const RegisterDesc ENS160_REGISTERS[] = {
    RegisterDesc(0x00, "PART_ID", 2, RegisterAccess::RO, 0x0160),
    RegisterDesc(0x10, "OPMODE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "CONFIG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "COMMAND", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x13, "TEMP_IN", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x15, "RH_IN", 2, RegisterAccess::RW, 0x0000),
    RegisterDesc(0x20, "DATA_STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x21, "DATA_AQI", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x22, "DATA_TVOC", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x24, "DATA_ECO2", 2, RegisterAccess::RO, 0x0000),
};

#define ENS160_REGISTER_COUNT (sizeof(ENS160_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* ENS160Driver::registers(size_t& count) const {
    count = ENS160_REGISTER_COUNT;
    return ENS160_REGISTERS;
}

bool ENS160Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf, len);
}

bool ENS160Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* ENS160Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < ENS160_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(ENS160_REGISTERS[i].name)) {
            return &ENS160_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
