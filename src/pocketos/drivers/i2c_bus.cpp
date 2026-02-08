#include "i2c_bus.h"

#ifdef POCKETOS_ENABLE_I2C

#include "../core/logger.h"

namespace PocketOS {
namespace Drivers {

I2CBus::I2CBus(int sda, int scl) : wire(&Wire), sda(sda), scl(scl) {
}

void I2CBus::init() {
    wire->begin(sda, scl);
    Logger::debug("I2C Bus initialized");
}

bool I2CBus::write(uint8_t address, uint8_t* data, size_t length) {
    wire->beginTransmission(address);
    wire->write(data, length);
    return wire->endTransmission() == 0;
}

bool I2CBus::read(uint8_t address, uint8_t* data, size_t length) {
    wire->requestFrom(address, length);
    size_t bytesRead = 0;
    while (wire->available() && bytesRead < length) {
        data[bytesRead++] = wire->read();
    }
    return bytesRead == length;
}

bool I2CBus::writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
    wire->beginTransmission(address);
    wire->write(reg);
    wire->write(value);
    return wire->endTransmission() == 0;
}

uint8_t I2CBus::readRegister(uint8_t address, uint8_t reg) {
    wire->beginTransmission(address);
    wire->write(reg);
    wire->endTransmission(false);
    wire->requestFrom(address, (uint8_t)1);
    if (wire->available()) {
        return wire->read();
    }
    return 0;
}

} // namespace Drivers
} // namespace PocketOS

#endif // POCKETOS_ENABLE_I2C
