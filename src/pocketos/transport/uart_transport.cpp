#include "uart_transport.h"
#include "../core/logger.h"

namespace PocketOS {

UARTTransport::UARTTransport(uint8_t port_id) 
    : port_id_(port_id), initialized_(false), platform_handle_(nullptr) {
}

UARTTransport::~UARTTransport() {
    deinit();
}

UARTError UARTTransport::init(const UARTConfig& config) {
    if (initialized_) {
        deinit();
    }
    
    config_ = config;
    
    UARTError result = platformInit();
    if (result == UARTError::OK) {
        initialized_ = true;
        Logger::info("UART port %d initialized (TX=%d, RX=%d, baud=%d)", 
                     port_id_, config_.tx_pin, config_.rx_pin, config_.baud_rate);
    } else {
        Logger::error("UART port %d init failed", port_id_);
    }
    
    return result;
}

void UARTTransport::deinit() {
    if (initialized_) {
        platformDeinit();
        initialized_ = false;
        Logger::info("UART port %d deinitialized", port_id_);
    }
}

UARTError UARTTransport::write(const uint8_t* data, size_t length) {
    if (!initialized_) return UARTError::NOT_INITIALIZED;
    if (length == 0) return UARTError::OK;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    HardwareSerial* serial = (HardwareSerial*)platform_handle_;
    size_t written = serial->write(data, length);
    
    if (written != length) {
        Logger::warn("UART%d write incomplete: %d/%d bytes", port_id_, written, length);
        return UARTError::BUFFER_OVERFLOW;
    }
#endif
    
    return UARTError::OK;
}

UARTError UARTTransport::write(const char* str) {
    return write((const uint8_t*)str, strlen(str));
}

UARTError UARTTransport::writeByte(uint8_t byte) {
    return write(&byte, 1);
}

UARTError UARTTransport::read(uint8_t* data, size_t length, size_t* bytes_read, uint32_t timeout_ms) {
    if (!initialized_) return UARTError::NOT_INITIALIZED;
    
    *bytes_read = 0;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    HardwareSerial* serial = (HardwareSerial*)platform_handle_;
    
    unsigned long start_time = millis();
    
    while (*bytes_read < length) {
        if (serial->available() > 0) {
            data[*bytes_read] = serial->read();
            (*bytes_read)++;
        } else {
            if (millis() - start_time > timeout_ms) {
                if (*bytes_read == 0) {
                    return UARTError::TIMEOUT;
                }
                break;
            }
            delay(1);
        }
    }
#endif
    
    return UARTError::OK;
}

UARTError UARTTransport::readByte(uint8_t* byte, uint32_t timeout_ms) {
    size_t bytes_read;
    return read(byte, 1, &bytes_read, timeout_ms);
}

UARTError UARTTransport::readUntil(uint8_t* buffer, size_t max_length, char terminator,
                                    size_t* bytes_read, uint32_t timeout_ms) {
    if (!initialized_) return UARTError::NOT_INITIALIZED;
    
    *bytes_read = 0;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    HardwareSerial* serial = (HardwareSerial*)platform_handle_;
    
    unsigned long start_time = millis();
    
    while (*bytes_read < max_length) {
        if (serial->available() > 0) {
            uint8_t byte = serial->read();
            buffer[*bytes_read] = byte;
            (*bytes_read)++;
            
            if (byte == (uint8_t)terminator) {
                return UARTError::OK;
            }
        } else {
            if (millis() - start_time > timeout_ms) {
                return (*bytes_read > 0) ? UARTError::OK : UARTError::TIMEOUT;
            }
            delay(1);
        }
    }
    
    return UARTError::BUFFER_OVERFLOW;
#else
    return UARTError::NOT_INITIALIZED;
#endif
}

size_t UARTTransport::available() const {
    if (!initialized_) return 0;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    HardwareSerial* serial = (HardwareSerial*)platform_handle_;
    return serial->available();
#else
    return 0;
#endif
}

void UARTTransport::flush() {
    if (!initialized_) return;
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    HardwareSerial* serial = (HardwareSerial*)platform_handle_;
    serial->flush();
#endif
}

// Platform-specific initialization
UARTError UARTTransport::platformInit() {
#if defined(ARDUINO_ARCH_ESP32)
    // ESP32: Serial0, Serial1, Serial2
    HardwareSerial* serial;
    
    switch (port_id_) {
        case 0: serial = &Serial; break;
        case 1: serial = &Serial1; break;
        case 2: serial = &Serial2; break;
        default:
            Logger::error("ESP32 supports UART ports 0-2");
            return UARTError::INVALID_CONFIG;
    }
    
    platform_handle_ = serial;
    
    // Convert config to ESP32 format
    uint32_t serial_config = SERIAL_8N1;  // Default
    
    // Data bits
    switch (config_.data_bits) {
        case UARTDataBits::BITS_5: serial_config = SERIAL_5N1; break;
        case UARTDataBits::BITS_6: serial_config = SERIAL_6N1; break;
        case UARTDataBits::BITS_7: serial_config = SERIAL_7N1; break;
        case UARTDataBits::BITS_8: serial_config = SERIAL_8N1; break;
    }
    
    // Parity
    if (config_.parity == UARTParity::EVEN) {
        serial_config = (config_.data_bits == UARTDataBits::BITS_8) ? SERIAL_8E1 : SERIAL_7E1;
    } else if (config_.parity == UARTParity::ODD) {
        serial_config = (config_.data_bits == UARTDataBits::BITS_8) ? SERIAL_8O1 : SERIAL_7O1;
    }
    
    // Stop bits (2 stop bits)
    if (config_.stop_bits == UARTStopBits::BITS_2) {
        serial_config = (serial_config & ~SERIAL_STOP_BIT_MASK) | SERIAL_STOP_BIT_2;
    }
    
    serial->begin(config_.baud_rate, serial_config, config_.rx_pin, config_.tx_pin);
    
    return UARTError::OK;
    
#elif defined(ARDUINO_ARCH_ESP8266)
    // ESP8266: Serial (UART0), Serial1 (UART1, TX only)
    HardwareSerial* serial;
    
    switch (port_id_) {
        case 0: serial = &Serial; break;
        case 1: serial = &Serial1; break;
        default:
            Logger::error("ESP8266 supports UART ports 0-1");
            return UARTError::INVALID_CONFIG;
    }
    
    platform_handle_ = serial;
    
    // ESP8266 UART config
    SerialConfig serial_config = SERIAL_8N1;
    
    if (config_.parity == UARTParity::EVEN) {
        serial_config = (config_.data_bits == UARTDataBits::BITS_8) ? SERIAL_8E1 : SERIAL_7E1;
    } else if (config_.parity == UARTParity::ODD) {
        serial_config = (config_.data_bits == UARTDataBits::BITS_8) ? SERIAL_8O1 : SERIAL_7O1;
    }
    
    serial->begin(config_.baud_rate, serial_config);
    
    return UARTError::OK;
    
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040: Serial1 (UART0), Serial2 (UART1)
    HardwareSerial* serial;
    
    switch (port_id_) {
        case 0: serial = &Serial1; break;
        case 1: serial = &Serial2; break;
        default:
            Logger::error("RP2040 supports UART ports 0-1");
            return UARTError::INVALID_CONFIG;
    }
    
    platform_handle_ = serial;
    
    serial->setTX(config_.tx_pin);
    serial->setRX(config_.rx_pin);
    serial->begin(config_.baud_rate);
    
    // RP2040 doesn't support advanced config via begin(), would need lower-level API
    
    return UARTError::OK;
    
#else
    Logger::error("UART not supported on this platform");
    return UARTError::NOT_INITIALIZED;
#endif
}

void UARTTransport::platformDeinit() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_RP2040)
    if (platform_handle_) {
        HardwareSerial* serial = (HardwareSerial*)platform_handle_;
        serial->end();
        platform_handle_ = nullptr;
    }
#endif
}

} // namespace PocketOS
