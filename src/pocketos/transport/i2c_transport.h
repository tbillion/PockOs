#ifndef POCKETOS_I2C_TRANSPORT_H
#define POCKETOS_I2C_TRANSPORT_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace PocketOS {

// I2C error codes
enum class I2CError {
    OK = 0,
    TIMEOUT,
    NACK,
    BUS_ERROR,
    INVALID_PIN,
    NOT_INITIALIZED,
    INVALID_ADDRESS,
    BUFFER_OVERFLOW
};

// I2C mode
enum class I2CMode {
    MASTER,
    SLAVE
};

// I2C bus configuration
struct I2CConfig {
    uint8_t sda_pin;
    uint8_t scl_pin;
    uint32_t speed_hz;  // 100000 (standard), 400000 (fast), 1000000 (fast+)
    I2CMode mode;
    uint8_t slave_address;  // Only for slave mode
    
    I2CConfig() : sda_pin(21), scl_pin(22), speed_hz(400000), 
                  mode(I2CMode::MASTER), slave_address(0) {}
};

// I2C Transport Interface
class I2CTransport {
public:
    I2CTransport(uint8_t bus_id = 0);
    ~I2CTransport();
    
    // Initialization
    I2CError init(const I2CConfig& config);
    void deinit();
    bool isInitialized() const { return initialized_; }
    
    // Master mode operations
    I2CError scan(uint8_t* found_addresses, uint8_t max_count, uint8_t* count);
    I2CError write(uint8_t address, const uint8_t* data, size_t length);
    I2CError read(uint8_t address, uint8_t* data, size_t length);
    I2CError writeRead(uint8_t address, const uint8_t* write_data, size_t write_len,
                       uint8_t* read_data, size_t read_len);
    I2CError writeRegister(uint8_t address, uint8_t reg, uint8_t value);
    I2CError readRegister(uint8_t address, uint8_t reg, uint8_t* value);
    I2CError readRegisters(uint8_t address, uint8_t reg, uint8_t* data, size_t length);
    
    // Slave mode operations (where supported)
    I2CError setSlaveReceiveCallback(void (*callback)(uint8_t*, size_t));
    I2CError setSlaveRequestCallback(void (*callback)());
    
    // Status
    const I2CConfig& getConfig() const { return config_; }
    uint8_t getBusId() const { return bus_id_; }
    
private:
    uint8_t bus_id_;
    I2CConfig config_;
    bool initialized_;
    void* platform_handle_;  // Platform-specific handle (TwoWire*, etc.)
    
    I2CError platformInit();
    void platformDeinit();
};

} // namespace PocketOS

#endif // POCKETOS_I2C_TRANSPORT_H
