#include "gpio_din.h"
#include <Arduino.h>
#include "../core/logger.h"

namespace PocketOS {
namespace Drivers {

GPIODigitalIn::GPIODigitalIn(int pin, bool pullup) : pin(pin), pullup(pullup) {
}

void GPIODigitalIn::init() {
    pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
    Logger::debug("GPIO Digital In initialized");
}

bool GPIODigitalIn::read() {
    return digitalRead(pin) == HIGH;
}

} // namespace Drivers
} // namespace PocketOS
