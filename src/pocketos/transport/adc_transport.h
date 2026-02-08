#ifndef POCKETOS_ADC_TRANSPORT_H
#define POCKETOS_ADC_TRANSPORT_H

#include "transport_base.h"

namespace PocketOS {

/**
 * ADC Transport
 * 
 * Analog-to-digital conversion transport for all platforms.
 * Provides unified interface for analog input reading.
 */
class ADCTransport : public TransportBase {
public:
    ADCTransport(const char* name);
    virtual ~ADCTransport();
    
    // Lifecycle
    bool init() override;
    bool deinit() override;
    bool isSupported() const override;
    
    // ADC operations
    uint16_t read(uint8_t channel);
    float readVoltage(uint8_t channel);
    
    // Configuration
    bool setResolution(uint8_t bits);
    uint8_t getResolution() const;
    
    // Validation
    bool isValidChannel(uint8_t channel) const;
    uint8_t getChannelCount() const;
    
    // Platform-specific info
    std::string getCapabilities() const override;
    
private:
    uint8_t resolution_;
    
    // Platform-specific helpers
    uint8_t getPlatformChannelCount() const;
    float getPlatformVref() const;
};

} // namespace PocketOS

#endif // POCKETOS_ADC_TRANSPORT_H
