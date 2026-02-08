#ifndef POCKETOS_UART_TRANSPORT_H
#define POCKETOS_UART_TRANSPORT_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace PocketOS {

// UART error codes
enum class UARTError {
    OK = 0,
    TIMEOUT,
    BUS_ERROR,
    INVALID_PIN,
    NOT_INITIALIZED,
    INVALID_CONFIG,
    BUFFER_OVERFLOW
};

// UART data bits
enum class UARTDataBits {
    BITS_5 = 5,
    BITS_6 = 6,
    BITS_7 = 7,
    BITS_8 = 8
};

// UART parity
enum class UARTParity {
    NONE,
    EVEN,
    ODD
};

// UART stop bits
enum class UARTStopBits {
    BITS_1,
    BITS_2
};

// UART configuration
struct UARTConfig {
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint32_t baud_rate;
    UARTDataBits data_bits;
    UARTParity parity;
    UARTStopBits stop_bits;
    size_t rx_buffer_size;
    size_t tx_buffer_size;
    
    UARTConfig() : tx_pin(1), rx_pin(3), baud_rate(115200),
                   data_bits(UARTDataBits::BITS_8), parity(UARTParity::NONE),
                   stop_bits(UARTStopBits::BITS_1),
                   rx_buffer_size(256), tx_buffer_size(256) {}
};

// UART Transport Interface
class UARTTransport {
public:
    UARTTransport(uint8_t port_id = 0);
    ~UARTTransport();
    
    // Initialization
    UARTError init(const UARTConfig& config);
    void deinit();
    bool isInitialized() const { return initialized_; }
    
    // Write operations
    UARTError write(const uint8_t* data, size_t length);
    UARTError write(const char* str);
    UARTError writeByte(uint8_t byte);
    
    // Read operations
    UARTError read(uint8_t* data, size_t length, size_t* bytes_read, uint32_t timeout_ms = 1000);
    UARTError readByte(uint8_t* byte, uint32_t timeout_ms = 1000);
    UARTError readUntil(uint8_t* buffer, size_t max_length, char terminator,
                        size_t* bytes_read, uint32_t timeout_ms = 1000);
    
    // Buffer status
    size_t available() const;
    void flush();
    
    // Status
    const UARTConfig& getConfig() const { return config_; }
    uint8_t getPortId() const { return port_id_; }
    
private:
    uint8_t port_id_;
    UARTConfig config_;
    bool initialized_;
    void* platform_handle_;  // Platform-specific handle (HardwareSerial*, etc.)
    
    UARTError platformInit();
    void platformDeinit();
};

} // namespace PocketOS

#endif // POCKETOS_UART_TRANSPORT_H
