#ifndef POCKETOS_GPIO_DOUT_H
#define POCKETOS_GPIO_DOUT_H

namespace PocketOS {
namespace Drivers {

class GPIODigitalOut {
private:
    int pin;
    bool state;

public:
    GPIODigitalOut(int pin);
    void init();
    void write(bool value);
    bool read();
    void toggle();
};

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_GPIO_DOUT_H
