#ifndef POCKETOS_SPI_TRANSPORT_H
#define POCKETOS_SPI_TRANSPORT_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace PocketOS {

// SPI error codes
enum class SPIError {
    OK = 0,
    TIMEOUT,
    BUS_ERROR,
    INVALID_PIN,
    NOT_INITIALIZED,
    INVALID_CONFIG
};

// SPI mode (clock polarity and phase)
enum class SPIMode {
    MODE0 = 0,  // CPOL=0, CPHA=0
    MODE1 = 1,  // CPOL=0, CPHA=1
    MODE2 = 2,  // CPOL=1, CPHA=0
    MODE3 = 3   // CPOL=1, CPHA=1
};

// SPI bit order
enum class SPIBitOrder {
    MSB_FIRST,
    LSB_FIRST
};

// SPI bus configuration
struct SPIConfig {
    uint8_t mosi_pin;
    uint8_t miso_pin;
    uint8_t sclk_pin;
    uint8_t cs_pin;      // Chip select (managed externally if 255)
    uint32_t speed_hz;   // Clock speed
    SPIMode mode;
    SPIBitOrder bit_order;
    
    SPIConfig() : mosi_pin(23), miso_pin(19), sclk_pin(18), cs_pin(255),
                  speed_hz(1000000), mode(SPIMode::MODE0), bit_order(SPIBitOrder::MSB_FIRST) {}
};

// SPI Transport Interface
class SPITransport {
public:
    SPITransport(uint8_t bus_id = 0);
    ~SPITransport();
    
    // Initialization
    SPIError init(const SPIConfig& config);
    void deinit();
    bool isInitialized() const { return initialized_; }
    
    // Data transfer operations
    SPIError transfer(uint8_t* data, size_t length);  // In-place transfer
    SPIError write(const uint8_t* data, size_t length);
    SPIError read(uint8_t* data, size_t length);
    SPIError writeRead(const uint8_t* write_data, size_t write_len,
                       uint8_t* read_data, size_t read_len);
    
    // Transaction management
    void beginTransaction();
    void endTransaction();
    
    // CS control (if managed by transport)
    void setCS(bool active);
    
    // Status
    const SPIConfig& getConfig() const { return config_; }
    uint8_t getBusId() const { return bus_id_; }
    
private:
    uint8_t bus_id_;
    SPIConfig config_;
    bool initialized_;
    bool in_transaction_;
    void* platform_handle_;  // Platform-specific handle (SPIClass*, etc.)
    
    SPIError platformInit();
    void platformDeinit();
};

} // namespace PocketOS

#endif // POCKETOS_SPI_TRANSPORT_H
