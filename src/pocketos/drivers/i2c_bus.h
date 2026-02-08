#ifndef POCKETOS_I2C_BUS_H
#define POCKETOS_I2C_BUS_H

#ifdef POCKETOS_ENABLE_I2C

#include <Wire.h>

namespace PocketOS {
namespace Drivers {

class I2CBus {
private:
    TwoWire* wire;
    int sda;
    int scl;

public:
    I2CBus(int sda = 21, int scl = 22);
    void init();
    bool write(uint8_t address, uint8_t* data, size_t length);
    bool read(uint8_t address, uint8_t* data, size_t length);
    bool writeRegister(uint8_t address, uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t address, uint8_t reg);
};

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_ENABLE_I2C

#endif // POCKETOS_I2C_BUS_H
