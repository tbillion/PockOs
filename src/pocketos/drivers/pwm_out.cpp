#include "pwm_out.h"

#ifdef POCKETOS_ENABLE_PWM

#include <Arduino.h>
#include "../core/logger.h"

namespace PocketOS {
namespace Drivers {

PWMOut::PWMOut(int pin, int channel, int frequency, int resolution) 
    : pin(pin), channel(channel), frequency(frequency), resolution(resolution) {
}

void PWMOut::init() {
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
    Logger::debug("PWM Out initialized");
}

void PWMOut::write(int dutyCycle) {
    ledcWrite(channel, dutyCycle);
}

void PWMOut::writePct(float percent) {
    int maxValue = (1 << resolution) - 1;
    int dutyCycle = (int)(maxValue * percent / 100.0);
    write(dutyCycle);
}

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_ENABLE_PWM
