#include "gpio_dout_driver.h"
#include "../core/logger.h"

namespace PocketOS {

GPIODoutDriver::GPIODoutDriver(const String& ep) : endpoint(ep), pin(-1), state(false) {
    // Parse pin from endpoint (e.g., "gpio.dout.2" -> pin 2)
    if (endpoint.startsWith("gpio.dout.")) {
        pin = endpoint.substring(10).toInt();
    }
}

GPIODoutDriver::~GPIODoutDriver() {
    if (pin >= 0) {
        ResourceManager::release(ResourceType::GPIO_PIN, pin, "gpio.dout." + String(pin));
    }
}

bool GPIODoutDriver::init() {
    if (pin < 0) {
        Logger::error("Invalid pin for GPIO DOUT driver");
        return false;
    }
    
    // Claim the pin resource
    if (!ResourceManager::claim(ResourceType::GPIO_PIN, pin, "gpio.dout." + String(pin))) {
        Logger::error("Failed to claim GPIO pin");
        return false;
    }
    
    // Initialize pin as output
    HAL::gpioMode(pin, OUTPUT);
    HAL::gpioWrite(pin, false);
    state = false;
    
    Logger::info(("GPIO DOUT driver initialized on pin " + String(pin)).c_str());
    return true;
}

bool GPIODoutDriver::setParam(const String& name, const String& value) {
    if (name == "state") {
        int val = value.toInt();
        state = (val != 0);
        HAL::gpioWrite(pin, state);
        return true;
    }
    return false;
}

String GPIODoutDriver::getParam(const String& name) {
    if (name == "state") {
        return String(state ? 1 : 0);
    } else if (name == "pin") {
        return String(pin);
    }
    return "";
}

CapabilitySchema GPIODoutDriver::getSchema() {
    CapabilitySchema schema;
    
    // Settings
    schema.addSetting("state", ParamType::BOOL, true, 0, 1, 1, "");
    schema.addSetting("pin", ParamType::INT, false, 0, 0, 0, "");
    
    // Signals
    schema.addSignal("output", ParamType::BOOL, false, "");
    
    // Commands
    schema.addCommand("toggle", "");
    
    return schema;
}

void GPIODoutDriver::update() {
    // Nothing to update for simple digital output
}

} // namespace PocketOS
