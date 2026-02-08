#ifndef POCKETOS_ONEWIRE_TRANSPORT_H
#define POCKETOS_ONEWIRE_TRANSPORT_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace PocketOS {

// OneWire error codes
enum class OneWireError {
    OK = 0,
    TIMEOUT,
    NO_DEVICE,
    CRC_ERROR,
    BUS_ERROR,
    INVALID_PIN,
    NOT_INITIALIZED
};

// OneWire device ROM (64-bit address)
struct OneWireROM {
    uint8_t family_code;
    uint8_t serial[6];
    uint8_t crc;
    
    bool isValid() const;
    void print() const;
};

// OneWire configuration
struct OneWireConfig {
    uint8_t pin;
    bool parasite_power;  // Enable parasite power mode
    
    OneWireConfig() : pin(4), parasite_power(false) {}
};

// OneWire Transport Interface
class OneWireTransport {
public:
    OneWireTransport();
    ~OneWireTransport();
    
    // Initialization
    OneWireError init(const OneWireConfig& config);
    void deinit();
    bool isInitialized() const { return initialized_; }
    
    // Bus operations
    bool reset();  // Reset bus and check for presence pulse
    void writeBit(bool bit);
    bool readBit();
    void writeByte(uint8_t byte);
    uint8_t readByte();
    
    // Device discovery
    OneWireError search(OneWireROM* roms, uint8_t max_devices, uint8_t* count);
    OneWireError searchNext(OneWireROM* rom, bool* found);
    void resetSearch();
    
    // Device selection
    void select(const OneWireROM& rom);
    void skip();  // Skip ROM (address all devices)
    
    // CRC calculation
    static uint8_t crc8(const uint8_t* data, size_t length);
    
    // Temperature sensor helpers (DS18B20, etc.)
    OneWireError startConversion(const OneWireROM* rom = nullptr);  // nullptr = all devices
    OneWireError readTemperature(const OneWireROM& rom, float* temperature);
    
    // Status
    const OneWireConfig& getConfig() const { return config_; }
    
private:
    OneWireConfig config_;
    bool initialized_;
    void* platform_handle_;  // Platform-specific handle (OneWire*, etc.)
    
    // Search state
    uint8_t last_discrepancy_;
    uint8_t last_family_discrepancy_;
    bool last_device_flag_;
    uint8_t rom_no_[8];
    
    OneWireError platformInit();
    void platformDeinit();
};

} // namespace PocketOS

#endif // POCKETOS_ONEWIRE_TRANSPORT_H
