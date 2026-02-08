#ifndef POCKETOS_GPIO_TRANSPORT_H
#define POCKETOS_GPIO_TRANSPORT_H

#include "transport_base.h"

namespace PocketOS {

/**
 * GPIO Transport
 * 
 * Digital input/output transport for all platforms.
 * Provides unified interface for GPIO operations.
 */
class GPIOTransport : public TransportBase {
public:
    // Pin modes
    enum class PinMode {
        INPUT,
        OUTPUT,
        INPUT_PULLUP,
        INPUT_PULLDOWN
    };
    
    // Pin state
    enum class PinState {
        LOW = 0,
        HIGH = 1
    };
    
    GPIOTransport(const char* name);
    virtual ~GPIOTransport();
    
    // Lifecycle
    bool init() override;
    bool deinit() override;
    bool isSupported() const override;
    
    // Pin configuration
    bool pinMode(uint8_t pin, PinMode mode);
    
    // Digital I/O
    bool digitalWrite(uint8_t pin, PinState state);
    PinState digitalRead(uint8_t pin);
    
    // Validation
    bool isValidPin(uint8_t pin) const;
    bool isPinConfigured(uint8_t pin) const;
    
    // Platform-specific info
    std::string getCapabilities() const override;
    
private:
    // Track configured pins
    std::map<uint8_t, PinMode> configuredPins_;
    
    // Platform-specific validation
    bool isPlatformValidPin(uint8_t pin) const;
};

} // namespace PocketOS

#endif // POCKETOS_GPIO_TRANSPORT_H
