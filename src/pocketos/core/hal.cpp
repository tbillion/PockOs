#include "hal.h"
#include <Arduino.h>
#include "logger.h"

namespace PocketOS {

void HAL::init() {
    Logger::info("HAL initialized");
}

void HAL::update() {
    // Update hardware abstraction layer
}

unsigned long HAL::getMillis() {
    return millis();
}

void HAL::delay(unsigned long ms) {
    ::delay(ms);
}

} // namespace PocketOS
