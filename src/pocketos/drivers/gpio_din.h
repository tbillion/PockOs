#ifndef POCKETOS_GPIO_DIN_H
#define POCKETOS_GPIO_DIN_H

namespace PocketOS {
namespace Drivers {

class GPIODigitalIn {
private:
    int pin;
    bool pullup;

public:
    GPIODigitalIn(int pin, bool pullup = false);
    void init();
    bool read();
};

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_GPIO_DIN_H
