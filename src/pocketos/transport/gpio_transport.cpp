#include "gpio_transport.h"
#include "../core/logger.h"

namespace PocketOS {

GPIOTransport::GPIOTransport(const char* name)
    : TransportBase(name, Type::GPIO, Tier::TIER0)
{
}

GPIOTransport::~GPIOTransport() {
    deinit();
}

bool GPIOTransport::init() {
    if (getState() == State::READY) {
        return true;
    }
    
    if (!isSupported()) {
        setError("GPIO not supported on this platform");
        return false;
    }
    
    setState(State::INITIALIZING);
    
    // GPIO is always available on Arduino platforms
    // No specific initialization needed
    
    setState(State::READY);
    Logger::log(Logger::Level::INFO, "GPIO transport initialized");
    return true;
}

bool GPIOTransport::deinit() {
    if (getState() == State::UNINITIALIZED) {
        return true;
    }
    
    // Reset all configured pins to input
    for (auto& pair : configuredPins_) {
        ::pinMode(pair.first, INPUT);
    }
    
    configuredPins_.clear();
    setState(State::UNINITIALIZED);
    
    Logger::log(Logger::Level::INFO, "GPIO transport deinitialized");
    return true;
}

bool GPIOTransport::isSupported() const {
    // GPIO is supported on all platforms
    return true;
}

bool GPIOTransport::pinMode(uint8_t pin, PinMode mode) {
    if (!isReady()) {
        return false;
    }
    
    if (!isValidPin(pin)) {
        return false;
    }
    
    // Set Arduino pin mode
    switch (mode) {
        case PinMode::INPUT:
            ::pinMode(pin, INPUT);
            break;
        case PinMode::OUTPUT:
            ::pinMode(pin, OUTPUT);
            break;
        case PinMode::INPUT_PULLUP:
            ::pinMode(pin, INPUT_PULLUP);
            break;
        case PinMode::INPUT_PULLDOWN:
#if defined(ESP32) || defined(ESP8266)
            ::pinMode(pin, INPUT_PULLDOWN);
#else
            // RP2040 doesn't have INPUT_PULLDOWN in Arduino core
            ::pinMode(pin, INPUT);
#endif
            break;
        default:
            return false;
    }
    
    configuredPins_[pin] = mode;
    incrementSuccess();
    return true;
}

bool GPIOTransport::digitalWrite(uint8_t pin, PinState state) {
    if (!isReady()) {
        return false;
    }
    
    if (!isPinConfigured(pin)) {
        return false;
    }
    
    ::digitalWrite(pin, state == PinState::HIGH ? HIGH : LOW);
    incrementSuccess();
    return true;
}

GPIOTransport::PinState GPIOTransport::digitalRead(uint8_t pin) {
    if (!isReady()) {
        return PinState::LOW;
    }
    
    if (!isPinConfigured(pin)) {
        return PinState::LOW;
    }
    
    incrementSuccess();
    return ::digitalRead(pin) == HIGH ? PinState::HIGH : PinState::LOW;
}

bool GPIOTransport::isValidPin(uint8_t pin) const {
    return isPlatformValidPin(pin);
}

bool GPIOTransport::isPinConfigured(uint8_t pin) const {
    return configuredPins_.find(pin) != configuredPins_.end();
}

bool GPIOTransport::isPlatformValidPin(uint8_t pin) const {
#if defined(ESP32)
    // ESP32 valid GPIO pins (excluding strapping/boot pins for safety)
    // Safe pins: 4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33
    if (pin == 0 || pin == 1 || pin == 2 || pin == 3 || 
        pin == 6 || pin == 7 || pin == 8 || pin == 9 || 
        pin == 10 || pin == 11 || 
        (pin >= 20 && pin <= 31 && pin != 21 && pin != 22 && 
         pin != 23 && pin != 25 && pin != 26 && pin != 27) || 
        pin >= 40) {
        return false;
    }
    return true;
#elif defined(ESP8266)
    // ESP8266 valid GPIO pins (D1 Mini pinout)
    // GPIO: 0,2,4,5,12,13,14,15,16
    // Safe pins: 4(D2),5(D1),12(D6),13(D7),14(D5)
    if (pin > 16) {
        return false;
    }
    // Avoid GPIO 1,3 (Serial), 6-11 (flash), 0,2,15 (boot/strapping)
    if (pin == 1 || pin == 3 || (pin >= 6 && pin <= 11)) {
        return false;
    }
    return true;
#elif defined(ARDUINO_ARCH_RP2040)
    // RP2040 Pico - GPIO 0-28 are valid (29 is special)
    return pin <= 28;
#else
    // Generic Arduino
    return pin < NUM_DIGITAL_PINS;
#endif
}

std::string GPIOTransport::getCapabilities() const {
#if defined(ESP32)
    return "ESP32: GPIO 4,5,12-19,21-23,25-27,32,33";
#elif defined(ESP8266)
    return "ESP8266: GPIO 0,2,4,5,12-16";
#elif defined(ARDUINO_ARCH_RP2040)
    return "RP2040: GPIO 0-28";
#else
    return "Generic GPIO";
#endif
}

} // namespace PocketOS
