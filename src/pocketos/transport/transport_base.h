#ifndef POCKETOS_TRANSPORT_BASE_H
#define POCKETOS_TRANSPORT_BASE_H

#include <Arduino.h>
#include <map>
#include <string>

namespace PocketOS {

/**
 * Transport Base Class
 * 
 * Unified interface for all transport layers in PocketOS.
 * Provides tier classification, state management, and lifecycle control.
 */
class TransportBase {
public:
    // Transport tier classification
    enum class Tier {
        TIER0,  // Basic hardware: GPIO, ADC, PWM, I2C, SPI, UART, OneWire
        TIER1,  // Hardware adapters: RS485, RS232, CAN, Ethernet, MCP2515, nRF24, LoRa, RC
        TIER2   // Radio/MAC surfaces: WiFi, BLE
    };
    
    // Transport state
    enum class State {
        UNINITIALIZED,  // Not yet initialized
        INITIALIZING,   // In progress
        READY,          // Initialized and operational
        ERROR,          // Error state
        DISABLED        // Explicitly disabled
    };
    
    // Transport type identifier
    enum class Type {
        // Tier 0
        GPIO,
        ADC,
        PWM,
        I2C,
        SPI,
        UART,
        ONEWIRE,
        // Tier 1
        RS485,
        RS232,
        CAN,
        ETHERNET,
        MCP2515,
        NRF24L01,
        LORAWAN,
        RC_TRANSCEIVER,
        // Tier 2
        WIFI,
        BLE
    };
    
    // Status information
    struct Status {
        State state;
        unsigned long lastError;
        unsigned long initTime;
        unsigned long uptime;
        uint32_t errorCount;
        uint32_t successCount;
        std::string lastErrorMsg;
    };
    
    // Constructor
    TransportBase(const char* name, Type type, Tier tier);
    virtual ~TransportBase();
    
    // Lifecycle
    virtual bool init() = 0;
    virtual bool deinit() = 0;
    virtual bool reset();
    
    // State management
    State getState() const { return state_; }
    bool isReady() const { return state_ == State::READY; }
    bool isError() const { return state_ == State::ERROR; }
    
    // Configuration
    virtual bool setConfig(const std::string& key, const std::string& value);
    virtual std::string getConfig(const std::string& key) const;
    virtual std::map<std::string, std::string> getAllConfig() const;
    
    // Status and diagnostics
    Status getStatus() const;
    const char* getName() const { return name_; }
    Type getType() const { return type_; }
    Tier getTier() const { return tier_; }
    
    // Capability detection
    virtual bool isSupported() const = 0;
    virtual std::string getCapabilities() const;
    
    // Type conversion helpers
    static const char* tierToString(Tier tier);
    static const char* stateToString(State state);
    static const char* typeToString(Type type);
    
protected:
    // State transition helpers
    void setState(State newState);
    void setError(const char* msg);
    void clearError();
    void incrementSuccess();
    
    // Configuration storage
    std::map<std::string, std::string> config_;
    
private:
    const char* name_;
    Type type_;
    Tier tier_;
    State state_;
    
    // Statistics
    unsigned long initTime_;
    uint32_t errorCount_;
    uint32_t successCount_;
    unsigned long lastErrorTime_;
    std::string lastErrorMsg_;
};

} // namespace PocketOS

#endif // POCKETOS_TRANSPORT_BASE_H
