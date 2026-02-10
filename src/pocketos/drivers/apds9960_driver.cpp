#include "apds9960_driver.h"
#include "../driver_config.h"

#if POCKETOS_APDS9960_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// APDS9960 Register addresses
#define APDS9960_REG_ENABLE       0x80
#define APDS9960_REG_ATIME        0x81
#define APDS9960_REG_WTIME        0x83
#define APDS9960_REG_PPULSE       0x8E
#define APDS9960_REG_CONTROL      0x8F
#define APDS9960_REG_ID           0x92
#define APDS9960_REG_STATUS       0x93
#define APDS9960_REG_CDATAL       0x94
#define APDS9960_REG_PDATA        0x9C
#define APDS9960_REG_GCONF1       0xA2
#define APDS9960_REG_GCONF4       0xAB
#define APDS9960_REG_GFLVL        0xAE
#define APDS9960_REG_GSTATUS      0xAF
#define APDS9960_REG_GFIFO_U      0xFC

#define APDS9960_ID               0xAB

APDS9960Driver::APDS9960Driver() : address(0), initialized(false), gestureMode(false) {}

bool APDS9960Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_APDS9960_ENABLE_LOGGING
    Logger::info("APDS9960: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t id = 0;
    if (!readRegister(APDS9960_REG_ID, &id)) {
#if POCKETOS_APDS9960_ENABLE_LOGGING
        Logger::error("APDS9960: Failed to read ID");
#endif
        return false;
    }
    
    if (id != APDS9960_ID) {
#if POCKETOS_APDS9960_ENABLE_LOGGING
        Logger::error("APDS9960: Invalid ID: 0x" + String(id, HEX));
#endif
        return false;
    }
    
    // Disable all features initially
    writeRegister(APDS9960_REG_ENABLE, 0x00);
    delay(10);
    
#if POCKETOS_APDS9960_ENABLE_CONFIGURATION
    // Configure timing
    writeRegister(APDS9960_REG_ATIME, 0xDB);  // 103ms
    writeRegister(APDS9960_REG_WTIME, 0xF6);  // 27ms
    
    // Configure proximity pulse
    writeRegister(APDS9960_REG_PPULSE, 0x87); // 8 pulses
    
    // Configure gain
    writeRegister(APDS9960_REG_CONTROL, 0x01); // Gain 4x
#endif
    
    // Enable power and RGBC/proximity
    writeRegister(APDS9960_REG_ENABLE, 0x07);
    
    initialized = true;
#if POCKETOS_APDS9960_ENABLE_LOGGING
    Logger::info("APDS9960: Initialized successfully");
#endif
    return true;
}

void APDS9960Driver::deinit() {
    if (initialized) {
        writeRegister(APDS9960_REG_ENABLE, 0x00);
    }
    initialized = false;
    gestureMode = false;
}

APDS9960ColorData APDS9960Driver::readColor() {
    APDS9960ColorData data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t buffer[8];
    if (!readBlock(APDS9960_REG_CDATAL, buffer, 8)) {
        return data;
    }
    
    data.clear = buffer[0] | (buffer[1] << 8);
    data.red = buffer[2] | (buffer[3] << 8);
    data.green = buffer[4] | (buffer[5] << 8);
    data.blue = buffer[6] | (buffer[7] << 8);
    data.valid = true;
    
    return data;
}

APDS9960ProximityData APDS9960Driver::readProximity() {
    APDS9960ProximityData data;
    
    if (!initialized) {
        return data;
    }
    
    if (!readRegister(APDS9960_REG_PDATA, &data.proximity)) {
        return data;
    }
    
    data.valid = true;
    return data;
}

GestureDirection APDS9960Driver::readGesture() {
    if (!initialized || !gestureMode) {
        enableGesture();
    }
    
    uint8_t status = 0;
    if (!readRegister(APDS9960_REG_GSTATUS, &status)) {
        return GestureDirection::NONE;
    }
    
    if (!(status & 0x01)) {
        return GestureDirection::NONE;
    }
    
    uint8_t fifo_level = 0;
    if (!readRegister(APDS9960_REG_GFLVL, &fifo_level)) {
        return GestureDirection::NONE;
    }
    
    if (fifo_level == 0) {
        return GestureDirection::NONE;
    }
    
    uint8_t fifo_data[128];
    if (!readBlock(APDS9960_REG_GFIFO_U, fifo_data, fifo_level * 4)) {
        return GestureDirection::NONE;
    }
    
    // Simple gesture detection based on first and last FIFO entries
    int16_t first_u = fifo_data[0];
    int16_t first_d = fifo_data[1];
    int16_t first_l = fifo_data[2];
    int16_t first_r = fifo_data[3];
    
    size_t last_idx = (fifo_level - 1) * 4;
    int16_t last_u = fifo_data[last_idx];
    int16_t last_d = fifo_data[last_idx + 1];
    int16_t last_l = fifo_data[last_idx + 2];
    int16_t last_r = fifo_data[last_idx + 3];
    
    int16_t ud_delta = last_u - first_u - (last_d - first_d);
    int16_t lr_delta = last_l - first_l - (last_r - first_r);
    
    if (abs(ud_delta) > abs(lr_delta)) {
        return (ud_delta > 0) ? GestureDirection::UP : GestureDirection::DOWN;
    } else if (abs(lr_delta) > 13) {
        return (lr_delta > 0) ? GestureDirection::LEFT : GestureDirection::RIGHT;
    }
    
    return GestureDirection::NONE;
}

bool APDS9960Driver::enableGesture() {
    if (!initialized) return false;
    
#if POCKETOS_APDS9960_ENABLE_CONFIGURATION
    writeRegister(APDS9960_REG_GCONF1, 0x40);
    writeRegister(APDS9960_REG_GCONF4, 0x01);
#endif
    
    uint8_t enable = 0;
    readRegister(APDS9960_REG_ENABLE, &enable);
    writeRegister(APDS9960_REG_ENABLE, enable | 0x45);
    
    gestureMode = true;
    return true;
}

bool APDS9960Driver::disableGesture() {
    if (!initialized) return false;
    
    uint8_t enable = 0;
    readRegister(APDS9960_REG_ENABLE, &enable);
    writeRegister(APDS9960_REG_ENABLE, enable & ~0x41);
    
    gestureMode = false;
    return true;
}

CapabilitySchema APDS9960Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "apds9960";
    schema.tier = POCKETOS_APDS9960_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("red", "Red light intensity", "16-bit", "0-65535"));
    schema.outputs.push_back(OutputDesc("green", "Green light intensity", "16-bit", "0-65535"));
    schema.outputs.push_back(OutputDesc("blue", "Blue light intensity", "16-bit", "0-65535"));
    schema.outputs.push_back(OutputDesc("clear", "Clear light intensity", "16-bit", "0-65535"));
    schema.outputs.push_back(OutputDesc("proximity", "Proximity level", "8-bit", "0-255"));
    schema.outputs.push_back(OutputDesc("gesture", "Gesture direction", "enum", "NONE/UP/DOWN/LEFT/RIGHT"));
    
    return schema;
}

String APDS9960Driver::getParameter(const String& name) {
    return "";
}

bool APDS9960Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool APDS9960Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    if (Wire.requestFrom(address, (uint8_t)1) != 1) {
        return false;
    }
    
    *value = Wire.read();
    return true;
}

bool APDS9960Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool APDS9960Driver::readBlock(uint8_t reg, uint8_t* buffer, size_t length) {
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

#if POCKETOS_APDS9960_ENABLE_REGISTER_ACCESS
static const RegisterDesc APDS9960_REGISTERS[] = {
    RegisterDesc(0x80, "ENABLE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x81, "ATIME", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x83, "WTIME", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x8E, "PPULSE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x8F, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x92, "ID", 1, RegisterAccess::RO, 0xAB),
    RegisterDesc(0x93, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x94, "CDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x95, "CDATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x96, "RDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x97, "RDATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x98, "GDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x99, "GDATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9A, "BDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9B, "BDATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x9C, "PDATA", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xA2, "GCONF1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xAB, "GCONF4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xAE, "GFLVL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xAF, "GSTATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xFC, "GFIFO_U", 1, RegisterAccess::RO, 0x00),
};

#define APDS9960_REGISTER_COUNT (sizeof(APDS9960_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* APDS9960Driver::registers(size_t& count) const {
    count = APDS9960_REGISTER_COUNT;
    return APDS9960_REGISTERS;
}

bool APDS9960Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf);
}

bool APDS9960Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* APDS9960Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < APDS9960_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(APDS9960_REGISTERS[i].name)) {
            return &APDS9960_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
