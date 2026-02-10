#include "ccs811_driver.h"
#include "../driver_config.h"

#if POCKETOS_CCS811_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// CCS811 Register addresses
#define CCS811_REG_STATUS         0x00
#define CCS811_REG_MEAS_MODE      0x01
#define CCS811_REG_ALG_RESULT     0x02
#define CCS811_REG_HW_ID          0x20
#define CCS811_REG_HW_VERSION     0x21
#define CCS811_REG_FW_BOOT_VER    0x23
#define CCS811_REG_FW_APP_VER     0x24
#define CCS811_REG_ERROR_ID       0xE0
#define CCS811_REG_APP_START      0xF4
#define CCS811_REG_SW_RESET       0xFF

#define CCS811_HW_ID              0x81

CCS811Driver::CCS811Driver() : address(0), initialized(false) {}

bool CCS811Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_CCS811_ENABLE_LOGGING
    Logger::info("CCS811: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t hwId = 0;
    if (!readRegister(CCS811_REG_HW_ID, &hwId, 1)) {
#if POCKETOS_CCS811_ENABLE_LOGGING
        Logger::error("CCS811: Failed to read hardware ID");
#endif
        return false;
    }
    
    if (hwId != CCS811_HW_ID) {
#if POCKETOS_CCS811_ENABLE_LOGGING
        Logger::error("CCS811: Invalid hardware ID: 0x" + String(hwId, HEX));
#endif
        return false;
    }
    
    // Check if application is valid and start it
    uint8_t status = 0;
    if (!readRegister(CCS811_REG_STATUS, &status, 1)) {
#if POCKETOS_CCS811_ENABLE_LOGGING
        Logger::error("CCS811: Failed to read status");
#endif
        return false;
    }
    
    if (!(status & 0x10)) {
#if POCKETOS_CCS811_ENABLE_LOGGING
        Logger::error("CCS811: No valid application");
#endif
        return false;
    }
    
    // Start application
    if (!writeRegister(CCS811_REG_APP_START)) {
#if POCKETOS_CCS811_ENABLE_LOGGING
        Logger::error("CCS811: Failed to start application");
#endif
        return false;
    }
    
    delay(100);
    
#if POCKETOS_CCS811_ENABLE_CONFIGURATION
    // Set measurement mode (Mode 1: every 1 second)
    Wire.beginTransmission(address);
    Wire.write(CCS811_REG_MEAS_MODE);
    Wire.write(0x10);
    if (Wire.endTransmission() != 0) {
#if POCKETOS_CCS811_ENABLE_LOGGING
        Logger::error("CCS811: Failed to set measurement mode");
#endif
        return false;
    }
#else
    // Minimal: just enable constant power mode
    Wire.beginTransmission(address);
    Wire.write(CCS811_REG_MEAS_MODE);
    Wire.write(0x10);
    Wire.endTransmission();
#endif
    
    initialized = true;
#if POCKETOS_CCS811_ENABLE_LOGGING
    Logger::info("CCS811: Initialized successfully");
#endif
    return true;
}

void CCS811Driver::deinit() {
    if (initialized) {
        Wire.beginTransmission(address);
        Wire.write(CCS811_REG_MEAS_MODE);
        Wire.write(0x00);
        Wire.endTransmission();
    }
    initialized = false;
}

CCS811Data CCS811Driver::readData() {
    CCS811Data data;
    
    if (!initialized) {
        return data;
    }
    
    if (!checkDataReady()) {
        return data;
    }
    
    uint8_t buffer[8];
    if (!readRegister(CCS811_REG_ALG_RESULT, buffer, 8)) {
        return data;
    }
    
    data.eco2 = (buffer[0] << 8) | buffer[1];
    data.tvoc = (buffer[2] << 8) | buffer[3];
    data.valid = true;
    
    return data;
}

bool CCS811Driver::checkDataReady() {
    uint8_t status = 0;
    if (!readRegister(CCS811_REG_STATUS, &status, 1)) {
        return false;
    }
    return (status & 0x08) != 0;
}

CapabilitySchema CCS811Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "ccs811";
    schema.tier = POCKETOS_CCS811_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("eco2", "Equivalent CO2", "ppm", "400-8192"));
    schema.outputs.push_back(OutputDesc("tvoc", "Total VOC", "ppb", "0-1187"));
    
    return schema;
}

String CCS811Driver::getParameter(const String& name) {
    return "";
}

bool CCS811Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool CCS811Driver::readRegister(uint8_t reg, uint8_t* buffer, size_t length) {
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

bool CCS811Driver::writeRegister(uint8_t reg) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    return Wire.endTransmission() == 0;
}

#if POCKETOS_CCS811_ENABLE_REGISTER_ACCESS
static const RegisterDesc CCS811_REGISTERS[] = {
    RegisterDesc(0x00, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "MEAS_MODE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "ALG_RESULT_DATA", 8, RegisterAccess::RO, 0x00),
    RegisterDesc(0x20, "HW_ID", 1, RegisterAccess::RO, 0x81),
    RegisterDesc(0x21, "HW_VERSION", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x23, "FW_BOOT_VERSION", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x24, "FW_APP_VERSION", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE0, "ERROR_ID", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0xF4, "APP_START", 0, RegisterAccess::WO, 0x00),
    RegisterDesc(0xFF, "SW_RESET", 4, RegisterAccess::WO, 0x00),
};

#define CCS811_REGISTER_COUNT (sizeof(CCS811_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* CCS811Driver::registers(size_t& count) const {
    count = CCS811_REGISTER_COUNT;
    return CCS811_REGISTERS;
}

bool CCS811Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf, len);
}

bool CCS811Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    
    Wire.beginTransmission(address);
    Wire.write((uint8_t)reg);
    for (size_t i = 0; i < len; i++) {
        Wire.write(buf[i]);
    }
    return Wire.endTransmission() == 0;
}

const RegisterDesc* CCS811Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < CCS811_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(CCS811_REGISTERS[i].name)) {
            return &CCS811_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
