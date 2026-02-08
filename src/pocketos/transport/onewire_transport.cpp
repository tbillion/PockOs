#include "onewire_transport.h"
#include "../core/logger.h"

#ifdef ARDUINO_ARCH_ESP32
#include <OneWire.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <OneWire.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include <OneWire.h>
#endif

namespace PocketOS {

// OneWire commands
#define ONEWIRE_CMD_SEARCH_ROM      0xF0
#define ONEWIRE_CMD_READ_ROM        0x33
#define ONEWIRE_CMD_MATCH_ROM       0x55
#define ONEWIRE_CMD_SKIP_ROM        0xCC
#define ONEWIRE_CMD_ALARM_SEARCH    0xEC

// DS18B20 commands
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

bool OneWireROM::isValid() const {
    return crc == OneWireTransport::crc8((const uint8_t*)this, 7);
}

void OneWireROM::print() const {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02X:%02X%02X%02X%02X%02X%02X:%02X",
             family_code, serial[0], serial[1], serial[2], serial[3], serial[4], serial[5], crc);
    Logger::info("OneWire ROM: %s", buffer);
}

OneWireTransport::OneWireTransport() 
    : initialized_(false), platform_handle_(nullptr),
      last_discrepancy_(0), last_family_discrepancy_(0), last_device_flag_(false) {
    memset(rom_no_, 0, 8);
}

OneWireTransport::~OneWireTransport() {
    deinit();
}

OneWireError OneWireTransport::init(const OneWireConfig& config) {
    if (initialized_) {
        deinit();
    }
    
    config_ = config;
    
    OneWireError result = platformInit();
    if (result == OneWireError::OK) {
        initialized_ = true;
        Logger::info("OneWire initialized on pin %d", config_.pin);
    } else {
        Logger::error("OneWire init failed on pin %d", config_.pin);
    }
    
    return result;
}

void OneWireTransport::deinit() {
    if (initialized_) {
        platformDeinit();
        initialized_ = false;
        Logger::info("OneWire deinitialized");
    }
}

bool OneWireTransport::reset() {
    if (!initialized_) return false;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    OneWire* ow = (OneWire*)platform_handle_;
    return ow->reset();
#else
    return false;
#endif
}

void OneWireTransport::writeBit(bool bit) {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    OneWire* ow = (OneWire*)platform_handle_;
    ow->write_bit(bit ? 1 : 0);
#endif
}

bool OneWireTransport::readBit() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    OneWire* ow = (OneWire*)platform_handle_;
    return ow->read_bit() != 0;
#else
    return false;
#endif
}

void OneWireTransport::writeByte(uint8_t byte) {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    OneWire* ow = (OneWire*)platform_handle_;
    ow->write(byte);
#endif
}

uint8_t OneWireTransport::readByte() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    OneWire* ow = (OneWire*)platform_handle_;
    return ow->read();
#else
    return 0;
#endif
}

void OneWireTransport::select(const OneWireROM& rom) {
    writeByte(ONEWIRE_CMD_MATCH_ROM);
    const uint8_t* addr = (const uint8_t*)&rom;
    for (int i = 0; i < 8; i++) {
        writeByte(addr[i]);
    }
}

void OneWireTransport::skip() {
    writeByte(ONEWIRE_CMD_SKIP_ROM);
}

void OneWireTransport::resetSearch() {
    last_discrepancy_ = 0;
    last_device_flag_ = false;
    last_family_discrepancy_ = 0;
    memset(rom_no_, 0, 8);
}

OneWireError OneWireTransport::searchNext(OneWireROM* rom, bool* found) {
    if (!initialized_) return OneWireError::NOT_INITIALIZED;
    
    *found = false;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    OneWire* ow = (OneWire*)platform_handle_;
    
    if (ow->search(rom_no_)) {
        if (crc8(rom_no_, 7) == rom_no_[7]) {
            memcpy(rom, rom_no_, 8);
            *found = true;
            return OneWireError::OK;
        } else {
            return OneWireError::CRC_ERROR;
        }
    }
    
    ow->reset_search();
#endif
    
    return OneWireError::OK;
}

OneWireError OneWireTransport::search(OneWireROM* roms, uint8_t max_devices, uint8_t* count) {
    if (!initialized_) return OneWireError::NOT_INITIALIZED;
    
    *count = 0;
    resetSearch();
    
    while (*count < max_devices) {
        bool found;
        OneWireError err = searchNext(&roms[*count], &found);
        if (err != OneWireError::OK) return err;
        if (!found) break;
        (*count)++;
    }
    
    return OneWireError::OK;
}

OneWireError OneWireTransport::startConversion(const OneWireROM* rom) {
    if (!initialized_) return OneWireError::NOT_INITIALIZED;
    
    if (!reset()) {
        return OneWireError::NO_DEVICE;
    }
    
    if (rom) {
        select(*rom);
    } else {
        skip();
    }
    
    writeByte(DS18B20_CMD_CONVERT_T);
    
    // Wait for conversion (750ms for DS18B20 at 12-bit resolution)
    if (config_.parasite_power) {
        // Strong pullup for parasite power
        delay(750);
    }
    
    return OneWireError::OK;
}

OneWireError OneWireTransport::readTemperature(const OneWireROM& rom, float* temperature) {
    if (!initialized_) return OneWireError::NOT_INITIALIZED;
    
    // Only works with DS18B20 family (0x28)
    if (rom.family_code != 0x28) {
        Logger::warn("OneWire device 0x%02X is not DS18B20", rom.family_code);
        return OneWireError::BUS_ERROR;
    }
    
    if (!reset()) {
        return OneWireError::NO_DEVICE;
    }
    
    select(rom);
    writeByte(DS18B20_CMD_READ_SCRATCHPAD);
    
    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        data[i] = readByte();
    }
    
    // Verify CRC
    if (crc8(data, 8) != data[8]) {
        Logger::warn("OneWire temperature read CRC error");
        return OneWireError::CRC_ERROR;
    }
    
    // Convert temperature (16-bit signed, 0.0625°C per bit)
    int16_t raw = (data[1] << 8) | data[0];
    *temperature = (float)raw * 0.0625f;
    
    return OneWireError::OK;
}

uint8_t OneWireTransport::crc8(const uint8_t* data, size_t length) {
    uint8_t crc = 0;
    
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        for (int bit = 0; bit < 8; bit++) {
            uint8_t mix = (crc ^ byte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            byte >>= 1;
        }
    }
    
    return crc;
}

// Platform-specific initialization
OneWireError OneWireTransport::platformInit() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    OneWire* ow = new OneWire(config_.pin);
    if (!ow) {
        Logger::error("Failed to allocate OneWire object");
        return OneWireError::BUS_ERROR;
    }
    
    platform_handle_ = ow;
    
    // Test bus
    if (!reset()) {
        Logger::warn("OneWire: No devices detected on pin %d", config_.pin);
        // Not an error - bus may be empty
    }
    
    return OneWireError::OK;
    
#else
    Logger::error("OneWire not supported on this platform");
    return OneWireError::NOT_INITIALIZED;
#endif
}

void OneWireTransport::platformDeinit() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    if (platform_handle_) {
        OneWire* ow = (OneWire*)platform_handle_;
        delete ow;
        platform_handle_ = nullptr;
    }
#endif
}

} // namespace PocketOS
