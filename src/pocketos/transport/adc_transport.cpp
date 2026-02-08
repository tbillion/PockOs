#include "adc_transport.h"
#include "../core/logger.h"

namespace PocketOS {

ADCTransport::ADCTransport(const char* name)
    : TransportBase(name, Type::ADC, Tier::TIER0)
    , resolution_(10)  // Default 10-bit
{
}

ADCTransport::~ADCTransport() {
    deinit();
}

bool ADCTransport::init() {
    if (getState() == State::READY) {
        return true;
    }
    
    if (!isSupported()) {
        setError("ADC not supported on this platform");
        return false;
    }
    
    setState(State::INITIALIZING);
    
#if defined(ESP32)
    // ESP32 ADC initialization
    analogReadResolution(resolution_);
    analogSetAttenuation(ADC_11db);  // 0-3.3V range
#elif defined(ESP8266)
    // ESP8266 has fixed 10-bit ADC
    resolution_ = 10;
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 ADC initialization
    analogReadResolution(resolution_);
#endif
    
    setState(State::READY);
    Logger::log(Logger::Level::INFO, "ADC transport initialized");
    return true;
}

bool ADCTransport::deinit() {
    if (getState() == State::UNINITIALIZED) {
        return true;
    }
    
    setState(State::UNINITIALIZED);
    Logger::log(Logger::Level::INFO, "ADC transport deinitialized");
    return true;
}

bool ADCTransport::isSupported() const {
    // ADC is supported on all platforms
    return true;
}

uint16_t ADCTransport::read(uint8_t channel) {
    if (!isReady()) {
        return 0;
    }
    
    if (!isValidChannel(channel)) {
        return 0;
    }
    
#if defined(ESP32)
    // ESP32: channels map to GPIO pins
    // ADC1: GPIO32-39 (channels 0-7)
    // ADC2: GPIO0,2,4,12-15,25-27 (avoid when WiFi active)
    uint8_t pins[] = {36, 39, 34, 35, 32, 33, 25, 26};
    if (channel < 8) {
        incrementSuccess();
        return analogRead(pins[channel]);
    }
    return 0;
#elif defined(ESP8266)
    // ESP8266: only one ADC channel (A0)
    if (channel == 0) {
        incrementSuccess();
        return analogRead(A0);
    }
    return 0;
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040: ADC channels 0-3 map to GPIO26-29
    if (channel < 4) {
        incrementSuccess();
        return analogRead(26 + channel);
    }
    return 0;
#else
    // Generic Arduino
    if (channel < NUM_ANALOG_INPUTS) {
        incrementSuccess();
        return analogRead(channel);
    }
    return 0;
#endif
}

float ADCTransport::readVoltage(uint8_t channel) {
    uint16_t raw = read(channel);
    float vref = getPlatformVref();
    uint16_t maxValue = (1 << resolution_) - 1;
    return (raw * vref) / maxValue;
}

bool ADCTransport::setResolution(uint8_t bits) {
    if (bits < 8 || bits > 16) {
        return false;
    }
    
#if defined(ESP32)
    // ESP32 supports 9-12 bits
    if (bits < 9 || bits > 12) {
        return false;
    }
    analogReadResolution(bits);
    resolution_ = bits;
    return true;
#elif defined(ESP8266)
    // ESP8266 has fixed 10-bit ADC
    return bits == 10;
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 supports 8-12 bits
    if (bits < 8 || bits > 12) {
        return false;
    }
    analogReadResolution(bits);
    resolution_ = bits;
    return true;
#else
    // Generic Arduino - usually 10-bit
    resolution_ = bits;
    return true;
#endif
}

uint8_t ADCTransport::getResolution() const {
    return resolution_;
}

bool ADCTransport::isValidChannel(uint8_t channel) const {
    return channel < getChannelCount();
}

uint8_t ADCTransport::getChannelCount() const {
    return getPlatformChannelCount();
}

uint8_t ADCTransport::getPlatformChannelCount() const {
#if defined(ESP32)
    return 8;  // ADC1 channels (ADC2 unreliable with WiFi)
#elif defined(ESP8266)
    return 1;  // Only A0
#elif defined(ARDUINO_ARCH_RP2040)
    return 4;  // ADC0-3 (GPIO26-29)
#else
    return NUM_ANALOG_INPUTS;
#endif
}

float ADCTransport::getPlatformVref() const {
#if defined(ESP32)
    return 3.3f;  // With ADC_11db attenuation
#elif defined(ESP8266)
    return 1.0f;  // ESP8266 ADC is 0-1V (with voltage divider to 3.3V input)
#elif defined(ARDUINO_ARCH_RP2040)
    return 3.3f;
#else
    return 5.0f;  // Standard Arduino
#endif
}

std::string ADCTransport::getCapabilities() const {
#if defined(ESP32)
    return "ESP32: 8 channels (ADC1), 12-bit, 0-3.3V";
#elif defined(ESP8266)
    return "ESP8266: 1 channel (A0), 10-bit, 0-1V";
#elif defined(ARDUINO_ARCH_RP2040)
    return "RP2040: 4 channels (ADC0-3), 12-bit, 0-3.3V";
#else
    return "Generic ADC";
#endif
}

} // namespace PocketOS
