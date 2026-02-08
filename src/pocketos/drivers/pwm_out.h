#ifndef POCKETOS_PWM_OUT_H
#define POCKETOS_PWM_OUT_H

#ifdef POCKETOS_ENABLE_PWM

namespace PocketOS {
namespace Drivers {

class PWMOut {
private:
    int pin;
    int channel;
    int frequency;
    int resolution;

public:
    PWMOut(int pin, int channel = 0, int frequency = 5000, int resolution = 8);
    void init();
    void write(int dutyCycle);
    void writePct(float percent);
};

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_ENABLE_PWM

#endif // POCKETOS_PWM_OUT_H
