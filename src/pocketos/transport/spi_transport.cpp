#include "spi_transport.h"
#include "../core/logger.h"

#ifdef ARDUINO_ARCH_ESP32
#include <SPI.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <SPI.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include <SPI.h>
#endif

namespace PocketOS {

SPITransport::SPITransport(uint8_t bus_id) 
    : bus_id_(bus_id), initialized_(false), in_transaction_(false), platform_handle_(nullptr) {
}

SPITransport::~SPITransport() {
    deinit();
}

SPIError SPITransport::init(const SPIConfig& config) {
    if (initialized_) {
        deinit();
    }
    
    config_ = config;
    
    SPIError result = platformInit();
    if (result == SPIError::OK) {
        initialized_ = true;
        Logger::info("SPI bus %d initialized (MOSI=%d, MISO=%d, SCLK=%d, speed=%d Hz)", 
                     bus_id_, config_.mosi_pin, config_.miso_pin, config_.sclk_pin, config_.speed_hz);
    } else {
        Logger::error("SPI bus %d init failed", bus_id_);
    }
    
    return result;
}

void SPITransport::deinit() {
    if (initialized_) {
        if (in_transaction_) {
            endTransaction();
        }
        platformDeinit();
        initialized_ = false;
        Logger::info("SPI bus %d deinitialized", bus_id_);
    }
}

void SPITransport::beginTransaction() {
    if (!initialized_) return;
    if (in_transaction_) return;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    SPIClass* spi = (SPIClass*)platform_handle_;
    
    uint8_t mode = static_cast<uint8_t>(config_.mode);
    uint8_t bit_order = (config_.bit_order == SPIBitOrder::MSB_FIRST) ? MSBFIRST : LSBFIRST;
    
    SPISettings settings(config_.speed_hz, bit_order, mode);
    spi->beginTransaction(settings);
    
    in_transaction_ = true;
    
    // Assert CS if managed
    if (config_.cs_pin != 255) {
        digitalWrite(config_.cs_pin, LOW);
    }
#endif
}

void SPITransport::endTransaction() {
    if (!initialized_) return;
    if (!in_transaction_) return;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    // Deassert CS if managed
    if (config_.cs_pin != 255) {
        digitalWrite(config_.cs_pin, HIGH);
    }
    
    SPIClass* spi = (SPIClass*)platform_handle_;
    spi->endTransaction();
    
    in_transaction_ = false;
#endif
}

void SPITransport::setCS(bool active) {
    if (config_.cs_pin == 255) return;
    
    digitalWrite(config_.cs_pin, active ? LOW : HIGH);
}

SPIError SPITransport::transfer(uint8_t* data, size_t length) {
    if (!initialized_) return SPIError::NOT_INITIALIZED;
    
    bool auto_transaction = !in_transaction_;
    if (auto_transaction) {
        beginTransaction();
    }
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    SPIClass* spi = (SPIClass*)platform_handle_;
    
    for (size_t i = 0; i < length; i++) {
        data[i] = spi->transfer(data[i]);
    }
#endif
    
    if (auto_transaction) {
        endTransaction();
    }
    
    return SPIError::OK;
}

SPIError SPITransport::write(const uint8_t* data, size_t length) {
    if (!initialized_) return SPIError::NOT_INITIALIZED;
    
    bool auto_transaction = !in_transaction_;
    if (auto_transaction) {
        beginTransaction();
    }
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    SPIClass* spi = (SPIClass*)platform_handle_;
    
    for (size_t i = 0; i < length; i++) {
        spi->transfer(data[i]);
    }
#endif
    
    if (auto_transaction) {
        endTransaction();
    }
    
    return SPIError::OK;
}

SPIError SPITransport::read(uint8_t* data, size_t length) {
    if (!initialized_) return SPIError::NOT_INITIALIZED;
    
    bool auto_transaction = !in_transaction_;
    if (auto_transaction) {
        beginTransaction();
    }
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    SPIClass* spi = (SPIClass*)platform_handle_;
    
    for (size_t i = 0; i < length; i++) {
        data[i] = spi->transfer(0xFF);  // Send dummy byte
    }
#endif
    
    if (auto_transaction) {
        endTransaction();
    }
    
    return SPIError::OK;
}

SPIError SPITransport::writeRead(const uint8_t* write_data, size_t write_len,
                                  uint8_t* read_data, size_t read_len) {
    beginTransaction();
    
    SPIError err = write(write_data, write_len);
    if (err != SPIError::OK) {
        endTransaction();
        return err;
    }
    
    err = read(read_data, read_len);
    endTransaction();
    
    return err;
}

// Platform-specific initialization
SPIError SPITransport::platformInit() {
#if defined(ARDUINO_ARCH_ESP32)
    // ESP32: VSPI (default) or HSPI
    SPIClass* spi;
    if (bus_id_ == 0) {
        spi = &SPI;  // VSPI
    } else if (bus_id_ == 1) {
        spi = new SPIClass(HSPI);
    } else {
        Logger::error("ESP32 only supports SPI bus 0 (VSPI) or 1 (HSPI)");
        return SPIError::INVALID_CONFIG;
    }
    
    platform_handle_ = spi;
    spi->begin(config_.sclk_pin, config_.miso_pin, config_.mosi_pin);
    
    // Setup CS pin if managed
    if (config_.cs_pin != 255) {
        pinMode(config_.cs_pin, OUTPUT);
        digitalWrite(config_.cs_pin, HIGH);
    }
    
    return SPIError::OK;
    
#elif defined(ARDUINO_ARCH_ESP8266)
    // ESP8266: Only one SPI bus
    if (bus_id_ != 0) {
        Logger::error("ESP8266 only supports SPI bus 0");
        return SPIError::INVALID_CONFIG;
    }
    
    platform_handle_ = &SPI;
    SPI.begin();
    
    // Setup CS pin if managed
    if (config_.cs_pin != 255) {
        pinMode(config_.cs_pin, OUTPUT);
        digitalWrite(config_.cs_pin, HIGH);
    }
    
    return SPIError::OK;
    
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040: SPI0 or SPI1
    SPIClass* spi = (bus_id_ == 0) ? &SPI : &SPI1;
    platform_handle_ = spi;
    
    spi->setRX(config_.miso_pin);
    spi->setTX(config_.mosi_pin);
    spi->setSCK(config_.sclk_pin);
    spi->begin();
    
    // Setup CS pin if managed
    if (config_.cs_pin != 255) {
        pinMode(config_.cs_pin, OUTPUT);
        digitalWrite(config_.cs_pin, HIGH);
    }
    
    return SPIError::OK;
    
#else
    Logger::error("SPI not supported on this platform");
    return SPIError::NOT_INITIALIZED;
#endif
}

void SPITransport::platformDeinit() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    if (platform_handle_) {
        SPIClass* spi = (SPIClass*)platform_handle_;
        spi->end();
        
#if defined(ARDUINO_ARCH_ESP32)
        if (bus_id_ == 1) {
            delete spi;  // HSPI was dynamically allocated
        }
#endif
        
        platform_handle_ = nullptr;
    }
#endif
}

} // namespace PocketOS
