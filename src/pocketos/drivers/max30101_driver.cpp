#include "max30101_driver.h"
#include "../driver_config.h"

#if POCKETOS_MAX30101_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define MAX30101_REG_INT_STATUS     0x00
#define MAX30101_REG_INT_ENABLE     0x02
#define MAX30101_REG_FIFO_WR_PTR    0x04
#define MAX30101_REG_FIFO_RD_PTR    0x06
#define MAX30101_REG_FIFO_DATA      0x07
#define MAX30101_REG_MODE_CONFIG    0x09
#define MAX30101_REG_SPO2_CONFIG    0x0A
#define MAX30101_REG_LED1_PA        0x0C
#define MAX30101_REG_LED2_PA        0x0D
#define MAX30101_REG_LED3_PA        0x0E
#define MAX30101_REG_PART_ID        0xFF

#define MAX30101_PART_ID            0x15

MAX30101Driver::MAX30101Driver() : address(0), initialized(false) {}

bool MAX30101Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MAX30101_ENABLE_LOGGING
    Logger::info("MAX30101: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t partId = 0;
    if (!readRegister(MAX30101_REG_PART_ID, &partId)) {
#if POCKETOS_MAX30101_ENABLE_LOGGING
        Logger::error("MAX30101: Failed to read part ID");
#endif
        return false;
    }
    
    if (partId != MAX30101_PART_ID) {
#if POCKETOS_MAX30101_ENABLE_LOGGING
        Logger::error("MAX30101: Invalid part ID: 0x" + String(partId, HEX));
#endif
        return false;
    }
    
    writeRegister(MAX30101_REG_MODE_CONFIG, 0x40);
    delay(50);
    
#if POCKETOS_MAX30101_ENABLE_CONFIGURATION
    writeRegister(MAX30101_REG_SPO2_CONFIG, 0x27);
    writeRegister(MAX30101_REG_LED1_PA, 0x24);
    writeRegister(MAX30101_REG_LED2_PA, 0x24);
    writeRegister(MAX30101_REG_LED3_PA, 0x24);
#endif
    
    writeRegister(MAX30101_REG_MODE_CONFIG, 0x07);
    
    initialized = true;
#if POCKETOS_MAX30101_ENABLE_LOGGING
    Logger::info("MAX30101: Initialized successfully");
#endif
    return true;
}

void MAX30101Driver::deinit() {
    if (initialized) {
        writeRegister(MAX30101_REG_MODE_CONFIG, 0x80);
    }
    initialized = false;
}

MAX30101Data MAX30101Driver::readData() {
    MAX30101Data data;
    
    if (!initialized) {
        return data;
    }
    
    uint8_t wrPtr = 0, rdPtr = 0;
    if (!readRegister(MAX30101_REG_FIFO_WR_PTR, &wrPtr)) {
        return data;
    }
    if (!readRegister(MAX30101_REG_FIFO_RD_PTR, &rdPtr)) {
        return data;
    }
    
    int8_t numSamples = wrPtr - rdPtr;
    if (numSamples < 0) numSamples += 32;
    
    if (numSamples > 0) {
        data.red = readFIFO();
        data.ir = readFIFO();
        data.green = readFIFO();
        data.valid = true;
    }
    
    return data;
}

uint32_t MAX30101Driver::readFIFO() {
    uint32_t value = 0;
    Wire.beginTransmission(address);
    Wire.write(MAX30101_REG_FIFO_DATA);
    Wire.endTransmission(false);
    
    Wire.requestFrom(address, (uint8_t)3);
    if (Wire.available() >= 3) {
        value = Wire.read();
        value = (value << 8) | Wire.read();
        value = (value << 8) | Wire.read();
        value &= 0x3FFFF;
    }
    
    return value;
}

CapabilitySchema MAX30101Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "max30101";
    schema.tier = POCKETOS_MAX30101_TIER_NAME;
    
    schema.outputs.push_back(OutputDesc("red", "Red LED PPG", "18-bit", "0-262143"));
    schema.outputs.push_back(OutputDesc("ir", "Infrared LED PPG", "18-bit", "0-262143"));
    schema.outputs.push_back(OutputDesc("green", "Green LED PPG", "18-bit", "0-262143"));
    
    return schema;
}

String MAX30101Driver::getParameter(const String& name) {
    return "";
}

bool MAX30101Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool MAX30101Driver::readRegister(uint8_t reg, uint8_t* value) {
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

bool MAX30101Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

#if POCKETOS_MAX30101_ENABLE_REGISTER_ACCESS
static const RegisterDesc MAX30101_REGISTERS[] = {
    RegisterDesc(0x00, "INT_STATUS_1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x01, "INT_STATUS_2", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "INT_ENABLE_1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "INT_ENABLE_2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x04, "FIFO_WR_PTR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "OVF_COUNTER", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "FIFO_RD_PTR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "FIFO_DATA", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x08, "FIFO_CONFIG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "MODE_CONFIG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "SPO2_CONFIG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "LED1_PA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "LED2_PA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "LED3_PA", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0xFF, "PART_ID", 1, RegisterAccess::RO, 0x15),
};

#define MAX30101_REGISTER_COUNT (sizeof(MAX30101_REGISTERS) / sizeof(RegisterDesc))

const RegisterDesc* MAX30101Driver::registers(size_t& count) const {
    count = MAX30101_REGISTER_COUNT;
    return MAX30101_REGISTERS;
}

bool MAX30101Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) {
        return false;
    }
    return readRegister((uint8_t)reg, buf);
}

bool MAX30101Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MAX30101Driver::findRegisterByName(const String& name) const {
    for (size_t i = 0; i < MAX30101_REGISTER_COUNT; i++) {
        if (name.equalsIgnoreCase(MAX30101_REGISTERS[i].name)) {
            return &MAX30101_REGISTERS[i];
        }
    }
    return nullptr;
}
#endif

} // namespace PocketOS
