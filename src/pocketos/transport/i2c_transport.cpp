#include "i2c_transport.h"
#include "../core/logger.h"

#ifdef ARDUINO_ARCH_ESP32
#include <Wire.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <Wire.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include <Wire.h>
#endif

namespace PocketOS {

I2CTransport::I2CTransport(uint8_t bus_id) 
    : bus_id_(bus_id), initialized_(false), platform_handle_(nullptr) {
}

I2CTransport::~I2CTransport() {
    deinit();
}

I2CError I2CTransport::init(const I2CConfig& config) {
    if (initialized_) {
        deinit();
    }
    
    config_ = config;
    
    I2CError result = platformInit();
    if (result == I2CError::OK) {
        initialized_ = true;
        Logger::info("I2C bus %d initialized (SDA=%d, SCL=%d, speed=%d Hz)", 
                     bus_id_, config_.sda_pin, config_.scl_pin, config_.speed_hz);
    } else {
        Logger::error("I2C bus %d init failed", bus_id_);
    }
    
    return result;
}

void I2CTransport::deinit() {
    if (initialized_) {
        platformDeinit();
        initialized_ = false;
        Logger::info("I2C bus %d deinitialized", bus_id_);
    }
}

I2CError I2CTransport::scan(uint8_t* found_addresses, uint8_t max_count, uint8_t* count) {
    if (!initialized_) return I2CError::NOT_INITIALIZED;
    
    *count = 0;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    TwoWire* wire = (TwoWire*)platform_handle_;
    
    for (uint8_t addr = 1; addr < 128 && *count < max_count; addr++) {
        wire->beginTransmission(addr);
        uint8_t error = wire->endTransmission();
        
        if (error == 0) {
            found_addresses[*count] = addr;
            (*count)++;
        }
    }
#endif
    
    return I2CError::OK;
}

I2CError I2CTransport::write(uint8_t address, const uint8_t* data, size_t length) {
    if (!initialized_) return I2CError::NOT_INITIALIZED;
    if (address >= 128) return I2CError::INVALID_ADDRESS;
    if (length == 0) return I2CError::OK;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    TwoWire* wire = (TwoWire*)platform_handle_;
    
    wire->beginTransmission(address);
    size_t written = wire->write(data, length);
    uint8_t result = wire->endTransmission();
    
    if (result != 0) {
        Logger::warn("I2C write to 0x%02X failed: %d", address, result);
        return (result == 2) ? I2CError::NACK : I2CError::BUS_ERROR;
    }
    
    if (written != length) {
        return I2CError::BUFFER_OVERFLOW;
    }
#endif
    
    return I2CError::OK;
}

I2CError I2CTransport::read(uint8_t address, uint8_t* data, size_t length) {
    if (!initialized_) return I2CError::NOT_INITIALIZED;
    if (address >= 128) return I2CError::INVALID_ADDRESS;
    if (length == 0) return I2CError::OK;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    TwoWire* wire = (TwoWire*)platform_handle_;
    
    size_t received = wire->requestFrom(address, (uint8_t)length);
    if (received != length) {
        Logger::warn("I2C read from 0x%02X: requested %d, got %d", address, length, received);
        return I2CError::NACK;
    }
    
    for (size_t i = 0; i < length; i++) {
        data[i] = wire->read();
    }
#endif
    
    return I2CError::OK;
}

I2CError I2CTransport::writeRead(uint8_t address, const uint8_t* write_data, size_t write_len,
                                  uint8_t* read_data, size_t read_len) {
    I2CError err = write(address, write_data, write_len);
    if (err != I2CError::OK) return err;
    
    return read(address, read_data, read_len);
}

I2CError I2CTransport::writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return write(address, data, 2);
}

I2CError I2CTransport::readRegister(uint8_t address, uint8_t reg, uint8_t* value) {
    I2CError err = write(address, &reg, 1);
    if (err != I2CError::OK) return err;
    
    return read(address, value, 1);
}

I2CError I2CTransport::readRegisters(uint8_t address, uint8_t reg, uint8_t* data, size_t length) {
    I2CError err = write(address, &reg, 1);
    if (err != I2CError::OK) return err;
    
    return read(address, data, length);
}

I2CError I2CTransport::setSlaveReceiveCallback(void (*callback)(uint8_t*, size_t)) {
    if (!initialized_) return I2CError::NOT_INITIALIZED;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_RP2040)
    // ESP32 and RP2040 support I2C slave mode
    TwoWire* wire = (TwoWire*)platform_handle_;
    wire->onReceive([](int numBytes) {
        // Note: This needs proper callback wrapper
    });
    return I2CError::OK;
#else
    // ESP8266 does not support I2C slave mode
    Logger::warn("I2C slave mode not supported on this platform");
    return I2CError::NOT_INITIALIZED;
#endif
}

I2CError I2CTransport::setSlaveRequestCallback(void (*callback)()) {
    if (!initialized_) return I2CError::NOT_INITIALIZED;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_RP2040)
    TwoWire* wire = (TwoWire*)platform_handle_;
    wire->onRequest(callback);
    return I2CError::OK;
#else
    Logger::warn("I2C slave mode not supported on this platform");
    return I2CError::NOT_INITIALIZED;
#endif
}

// Platform-specific initialization
I2CError I2CTransport::platformInit() {
#if defined(ARDUINO_ARCH_ESP32)
    // ESP32: Use Wire or Wire1
    TwoWire* wire = (bus_id_ == 0) ? &Wire : &Wire1;
    platform_handle_ = wire;
    
    if (config_.mode == I2CMode::MASTER) {
        wire->begin(config_.sda_pin, config_.scl_pin);
        wire->setClock(config_.speed_hz);
    } else {
        // Slave mode
        wire->begin(config_.slave_address, config_.sda_pin, config_.scl_pin);
    }
    
    return I2CError::OK;
    
#elif defined(ARDUINO_ARCH_ESP8266)
    // ESP8266: Only one I2C bus, master mode only
    if (bus_id_ != 0) {
        Logger::error("ESP8266 only supports I2C bus 0");
        return I2CError::INVALID_PIN;
    }
    
    if (config_.mode != I2CMode::MASTER) {
        Logger::error("ESP8266 only supports I2C master mode");
        return I2CError::NOT_INITIALIZED;
    }
    
    platform_handle_ = &Wire;
    Wire.begin(config_.sda_pin, config_.scl_pin);
    Wire.setClock(config_.speed_hz);
    
    return I2CError::OK;
    
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040: Wire (I2C0) or Wire1 (I2C1), both master and slave
    TwoWire* wire = (bus_id_ == 0) ? &Wire : &Wire1;
    platform_handle_ = wire;
    
    if (config_.mode == I2CMode::MASTER) {
        wire->setSDA(config_.sda_pin);
        wire->setSCL(config_.scl_pin);
        wire->begin();
        wire->setClock(config_.speed_hz);
    } else {
        // Slave mode
        wire->setSDA(config_.sda_pin);
        wire->setSCL(config_.scl_pin);
        wire->begin(config_.slave_address);
    }
    
    return I2CError::OK;
    
#else
    Logger::error("I2C not supported on this platform");
    return I2CError::NOT_INITIALIZED;
#endif
}

void I2CTransport::platformDeinit() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    if (platform_handle_) {
        TwoWire* wire = (TwoWire*)platform_handle_;
        wire->end();
        platform_handle_ = nullptr;
    }
#endif
}

} // namespace PocketOS
