#include "pwm_transport.h"
#include "../core/logger.h"

namespace PocketOS {

PWMTransport::PWMTransport(const char* name)
    : TransportBase(name, Type::PWM, Tier::TIER0)
    , resolution_(10)  // Default 10-bit (0-1023)
    , nextChannel_(0)
{
}

PWMTransport::~PWMTransport() {
    deinit();
}

bool PWMTransport::init() {
    if (getState() == State::READY) {
        return true;
    }
    
    if (!isSupported()) {
        setError("PWM not supported on this platform");
        return false;
    }
    
    setState(State::INITIALIZING);
    
#if defined(ESP32)
    // ESP32 LEDC initialization is done per-channel
    // Default resolution: 10-bit
    resolution_ = 10;
#elif defined(ESP8266)
    // ESP8266 PWM initialization
    resolution_ = 10;
    analogWriteRange((1 << resolution_) - 1);
    analogWriteFreq(1000);  // 1kHz default
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 PWM initialization
    resolution_ = 8;  // RP2040 typically uses 8-bit
    analogWriteResolution(resolution_);
#endif
    
    setState(State::READY);
    Logger::log(Logger::Level::INFO, "PWM transport initialized");
    return true;
}

bool PWMTransport::deinit() {
    if (getState() == State::UNINITIALIZED) {
        return true;
    }
    
    // Detach all pins
    for (auto& pair : attachedPins_) {
#if defined(ESP32)
        ledcDetachPin(pair.first);
#endif
    }
    
    attachedPins_.clear();
    nextChannel_ = 0;
    
    setState(State::UNINITIALIZED);
    Logger::log(Logger::Level::INFO, "PWM transport deinitialized");
    return true;
}

bool PWMTransport::isSupported() const {
    // PWM is supported on all platforms
    return true;
}

bool PWMTransport::attach(uint8_t pin, uint8_t channel, uint32_t frequency) {
    if (!isReady()) {
        return false;
    }
    
    if (!isValidPin(pin)) {
        return false;
    }
    
    if (isPinAttached(pin)) {
        detach(pin);
    }
    
    // Auto-assign channel if needed
    if (channel == 255) {
        channel = nextChannel_++;
        if (nextChannel_ >= getChannelCount()) {
            nextChannel_ = 0;
        }
    }
    
#if defined(ESP32)
    // ESP32 LEDC setup
    if (channel >= 16) {
        return false;
    }
    ledcSetup(channel, frequency, resolution_);
    ledcAttachPin(pin, channel);
#elif defined(ESP8266)
    // ESP8266 PWM
    pinMode(pin, OUTPUT);
    if (frequency > 0) {
        analogWriteFreq(frequency);
    }
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 PWM
    pinMode(pin, OUTPUT);
    if (frequency > 0) {
        analogWriteFreq(frequency);
    }
#else
    // Generic Arduino
    pinMode(pin, OUTPUT);
#endif
    
    PWMChannel pwmChannel;
    pwmChannel.channel = channel;
    pwmChannel.frequency = frequency;
    pwmChannel.resolution = resolution_;
    attachedPins_[pin] = pwmChannel;
    
    incrementSuccess();
    return true;
}

bool PWMTransport::detach(uint8_t pin) {
    if (!isPinAttached(pin)) {
        return false;
    }
    
#if defined(ESP32)
    ledcDetachPin(pin);
#endif
    
    attachedPins_.erase(pin);
    return true;
}

bool PWMTransport::write(uint8_t pin, uint16_t dutyCycle) {
    if (!isPinAttached(pin)) {
        return false;
    }
    
    uint16_t maxValue = (1 << resolution_) - 1;
    if (dutyCycle > maxValue) {
        dutyCycle = maxValue;
    }
    
#if defined(ESP32)
    auto& channel = attachedPins_[pin];
    ledcWrite(channel.channel, dutyCycle);
#else
    // ESP8266, RP2040, and generic Arduino
    analogWrite(pin, dutyCycle);
#endif
    
    incrementSuccess();
    return true;
}

bool PWMTransport::writePercent(uint8_t pin, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    
    uint16_t maxValue = (1 << resolution_) - 1;
    uint16_t dutyCycle = (uint16_t)((percent / 100.0f) * maxValue);
    
    return write(pin, dutyCycle);
}

bool PWMTransport::setFrequency(uint8_t pin, uint32_t frequency) {
    if (!isPinAttached(pin)) {
        return false;
    }
    
#if defined(ESP32)
    auto& channel = attachedPins_[pin];
    ledcSetup(channel.channel, frequency, resolution_);
    channel.frequency = frequency;
    return true;
#elif defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
    analogWriteFreq(frequency);
    attachedPins_[pin].frequency = frequency;
    return true;
#else
    // Generic Arduino - frequency setting not supported
    return false;
#endif
}

bool PWMTransport::setResolution(uint8_t bits) {
    if (bits < 8 || bits > 16) {
        return false;
    }
    
#if defined(ESP32)
    // ESP32 LEDC supports 1-16 bits
    if (bits > 16) {
        return false;
    }
    resolution_ = bits;
    // Update all attached channels
    for (auto& pair : attachedPins_) {
        ledcSetup(pair.second.channel, pair.second.frequency, resolution_);
    }
    return true;
#elif defined(ESP8266)
    // ESP8266 supports variable resolution
    resolution_ = bits;
    analogWriteRange((1 << resolution_) - 1);
    return true;
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 supports 8-16 bits
    if (bits < 8 || bits > 16) {
        return false;
    }
    resolution_ = bits;
    analogWriteResolution(bits);
    return true;
#else
    // Generic Arduino - typically fixed 8-bit
    return bits == 8;
#endif
}

uint8_t PWMTransport::getResolution() const {
    return resolution_;
}

bool PWMTransport::isValidPin(uint8_t pin) const {
    // Most GPIO pins support PWM on modern platforms
    // Use same validation as GPIO transport
#if defined(ESP32)
    // ESP32 - all output-capable pins support PWM via LEDC
    return pin < 40 && pin != 6 && pin != 7 && pin != 8 && 
           pin != 9 && pin != 10 && pin != 11 && pin != 20;
#elif defined(ESP8266)
    // ESP8266 - most GPIO pins support PWM
    return pin <= 16 && pin != 1 && pin != 3 && 
           !(pin >= 6 && pin <= 11);
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 - all GPIO pins support PWM
    return pin <= 28;
#else
    // Generic Arduino
    return true;
#endif
}

bool PWMTransport::isPinAttached(uint8_t pin) const {
    return attachedPins_.find(pin) != attachedPins_.end();
}

uint8_t PWMTransport::getChannelCount() const {
    return getPlatformChannelCount();
}

uint8_t PWMTransport::getPlatformChannelCount() const {
#if defined(ESP32)
    return 16;  // ESP32 LEDC has 16 channels
#elif defined(ESP8266)
    return 8;   // ESP8266 supports up to 8 PWM outputs
#elif defined(ARDUINO_ARCH_RP2040)
    return 16;  // RP2040 has 8 PWM slices × 2 channels
#else
    return 6;   // Generic Arduino (UNO-like)
#endif
}

uint32_t PWMTransport::getDefaultFrequency() const {
#if defined(ESP32)
    return 5000;  // 5kHz
#elif defined(ESP8266)
    return 1000;  // 1kHz
#elif defined(ARDUINO_ARCH_RP2040)
    return 1000;  // 1kHz
#else
    return 490;   // ~490Hz (Arduino default)
#endif
}

std::string PWMTransport::getCapabilities() const {
#if defined(ESP32)
    return "ESP32: 16 channels (LEDC), 1-16 bit, configurable frequency";
#elif defined(ESP8266)
    return "ESP8266: 8 channels, 10-bit, configurable frequency";
#elif defined(ARDUINO_ARCH_RP2040)
    return "RP2040: 16 channels (8 slices), 8-16 bit, configurable frequency";
#else
    return "Generic PWM";
#endif
}

} // namespace PocketOS
