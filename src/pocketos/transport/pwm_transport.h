#ifndef POCKETOS_PWM_TRANSPORT_H
#define POCKETOS_PWM_TRANSPORT_H

#include "transport_base.h"
#include <map>

namespace PocketOS {

/**
 * PWM Transport
 * 
 * Pulse-width modulation transport for all platforms.
 * Provides unified interface for PWM output generation.
 */
class PWMTransport : public TransportBase {
public:
    PWMTransport(const char* name);
    virtual ~PWMTransport();
    
    // Lifecycle
    bool init() override;
    bool deinit() override;
    bool isSupported() const override;
    
    // PWM operations
    bool attach(uint8_t pin, uint8_t channel, uint32_t frequency);
    bool detach(uint8_t pin);
    bool write(uint8_t pin, uint16_t dutyCycle);  // 0-1023 or 0-255 depending on resolution
    bool writePercent(uint8_t pin, float percent);  // 0.0-100.0%
    
    // Configuration
    bool setFrequency(uint8_t pin, uint32_t frequency);
    bool setResolution(uint8_t bits);
    uint8_t getResolution() const;
    
    // Validation
    bool isValidPin(uint8_t pin) const;
    bool isPinAttached(uint8_t pin) const;
    uint8_t getChannelCount() const;
    
    // Platform-specific info
    std::string getCapabilities() const override;
    
private:
    struct PWMChannel {
        uint8_t channel;
        uint32_t frequency;
        uint8_t resolution;
    };
    
    std::map<uint8_t, PWMChannel> attachedPins_;
    uint8_t resolution_;
    uint8_t nextChannel_;
    
    // Platform-specific helpers
    uint8_t getPlatformChannelCount() const;
    uint32_t getDefaultFrequency() const;
};

} // namespace PocketOS

#endif // POCKETOS_PWM_TRANSPORT_H
