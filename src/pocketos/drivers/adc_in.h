#ifndef POCKETOS_ADC_IN_H
#define POCKETOS_ADC_IN_H

#ifdef POCKETOS_ENABLE_ADC

namespace PocketOS {
namespace Drivers {

class ADCIn {
private:
    int pin;
    int resolution;

public:
    ADCIn(int pin, int resolution = 12);
    void init();
    int read();
    float readVoltage(float vref = 3.3);
};

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_ENABLE_ADC

#endif // POCKETOS_ADC_IN_H
