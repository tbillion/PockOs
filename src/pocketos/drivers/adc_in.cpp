#include "adc_in.h"

#ifdef POCKETOS_ENABLE_ADC

#include <Arduino.h>
#include "../core/logger.h"

namespace PocketOS {
namespace Drivers {

ADCIn::ADCIn(int pin, int resolution) : pin(pin), resolution(resolution), maxValue(0) {
}

void ADCIn::init() {
    maxValue = (1 << resolution) - 1;
    analogReadResolution(resolution);
    Logger::debug("ADC In initialized");
}

int ADCIn::read() {
    return analogRead(pin);
}

float ADCIn::readVoltage(float vref) {
    int value = read();
    return (value * vref) / maxValue;
}

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_ENABLE_ADC
