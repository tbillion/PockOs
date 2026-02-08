#include "devices.h"
#include "logger.h"

namespace PocketOS {

void Devices::init() {
    Logger::info("Devices initialized");
}

void Devices::registerDevice(const char* name) {
    Logger::debug("Device registered");
}

void Devices::unregisterDevice(const char* name) {
    Logger::debug("Device unregistered");
}

void Devices::listDevices() {
    Logger::debug("Listing devices");
}

} // namespace PocketOS
