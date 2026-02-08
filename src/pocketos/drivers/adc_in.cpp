#include "adc_in.h"

#ifdef POCKETOS_ENABLE_ADC

#include <Arduino.h>
#include "../core/logger.h"

namespace PocketOS {
namespace Drivers {

ADCIn::ADCIn(int pin, int resolution) : pin(pin), resolution(resolution) {
}

void ADCIn::init() {
    analogReadResolution(resolution);
    Logger::debug("ADC In initialized");
}

int ADCIn::read() {
    return analogRead(pin);
}

float ADCIn::readVoltage(float vref) {
    int value = read();
    int maxValue = (1 << resolution) - 1;
    return (value * vref) / maxValue;
}

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_ENABLE_ADC
