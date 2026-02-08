#ifndef POCKETOS_HAL_H
#define POCKETOS_HAL_H

namespace PocketOS {

class HAL {
public:
    static void init();
    static void update();
    static unsigned long getMillis();
    static void delay(unsigned long ms);
};

} // namespace PocketOS

#endif // POCKETOS_HAL_H
