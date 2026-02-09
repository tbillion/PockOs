#include "gpio_dout.h"
#include <Arduino.h>
#include "../core/logger.h"

namespace PocketOS {
namespace Drivers {

GPIODigitalOut::GPIODigitalOut(int pin) : pin(pin), state(false) {
}

void GPIODigitalOut::init() {
    pinMode(pin, OUTPUT);
    Logger::debug("GPIO Digital Out initialized");
}

void GPIODigitalOut::write(bool value) {
    state = value;
    digitalWrite(pin, value ? HIGH : LOW);
}

bool GPIODigitalOut::read() {
    return state;
}

void GPIODigitalOut::toggle() {
    write(!state);
}

} // namespace Drivers
} // namespace PocketOS
