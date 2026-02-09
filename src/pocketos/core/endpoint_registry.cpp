#include "endpoint_registry.h"
#include "hal.h"
#include "logger.h"

namespace PocketOS {

Endpoint EndpointRegistry::endpoints[MAX_ENDPOINTS];
int EndpointRegistry::endpointCount = 0;

void EndpointRegistry::init() {
    endpointCount = 0;
    for (int i = 0; i < MAX_ENDPOINTS; i++) {
        endpoints[i].active = false;
    }
    
    autoRegisterEndpoints();
    Logger::info("Endpoint Registry initialized");
}

void EndpointRegistry::autoRegisterEndpoints() {
    // Register I2C buses
    int i2cCount = HAL::getI2CCount();
    for (int i = 0; i < i2cCount; i++) {
        String addr = "i2c" + String(i);
        registerEndpoint(addr, EndpointType::I2C_BUS, i);
    }
    
    // GPIO pins registered on-demand
    // ADC channels registered on-demand
}

bool EndpointRegistry::registerEndpoint(const String& address, EndpointType type, int resourceId) {
    if (endpointExists(address)) {
        return true; // Already registered
    }
    
    int slot = findFreeSlot();
    if (slot < 0) {
        Logger::error("No free endpoint slots");
        return false;
    }
    
    endpoints[slot].active = true;
    endpoints[slot].address = address;
    endpoints[slot].type = type;
    endpoints[slot].resourceId = resourceId;
    endpointCount++;
    
    return true;
}

bool EndpointRegistry::unregisterEndpoint(const String& address) {
    int idx = findEndpoint(address);
    if (idx < 0) {
        return false;
    }
    
    endpoints[idx].active = false;
    endpointCount--;
    return true;
}

bool EndpointRegistry::endpointExists(const String& address) {
    return findEndpoint(address) >= 0;
}

EndpointType EndpointRegistry::getEndpointType(const String& address) {
    int idx = findEndpoint(address);
    if (idx >= 0) {
        return endpoints[idx].type;
    }
    return EndpointType::UNKNOWN;
}

int EndpointRegistry::getEndpointResource(const String& address) {
    int idx = findEndpoint(address);
    if (idx >= 0) {
        return endpoints[idx].resourceId;
    }
    return -1;
}

String EndpointRegistry::listEndpoints() {
    String result = "";
    for (int i = 0; i < MAX_ENDPOINTS; i++) {
        if (endpoints[i].active) {
            result += endpoints[i].address;
            result += " (" + String(endpointTypeToString(endpoints[i].type)) + ")";
            result += " [" + String(endpoints[i].resourceId) + "]\n";
        }
    }
    if (result.length() == 0) {
        result = "No endpoints registered\n";
    }
    return result;
}

String EndpointRegistry::probeEndpoint(const String& address) {
    // Check if endpoint is I2C bus
    if (address.startsWith("i2c")) {
        int busNum = address.substring(3).toInt();
        
#ifdef POCKETOS_ENABLE_I2C
        String result = "I2C" + String(busNum) + " scan:\n";
        
        // Scan I2C addresses 0x08-0x77
        int found = 0;
        for (uint8_t addr = 0x08; addr < 0x78; addr++) {
            if (HAL::i2cProbe(busNum, addr)) {
                result += "  0x" + String(addr, HEX) + "\n";
                found++;
            }
        }
        
        if (found == 0) {
            result += "  No devices found\n";
        }
        
        return result;
#else
        return "I2C not enabled\n";
#endif
    }
    
    return "";
}

int EndpointRegistry::findEndpoint(const String& address) {
    for (int i = 0; i < MAX_ENDPOINTS; i++) {
        if (endpoints[i].active && endpoints[i].address == address) {
            return i;
        }
    }
    return -1;
}

int EndpointRegistry::findFreeSlot() {
    for (int i = 0; i < MAX_ENDPOINTS; i++) {
        if (!endpoints[i].active) {
            return i;
        }
    }
    return -1;
}

const char* EndpointRegistry::endpointTypeToString(EndpointType type) {
    switch (type) {
        case EndpointType::GPIO_PIN: return "gpio.pin";
        case EndpointType::GPIO_DIN: return "gpio.din";
        case EndpointType::GPIO_DOUT: return "gpio.dout";
        case EndpointType::GPIO_PWM: return "gpio.pwm";
        case EndpointType::ADC_CH: return "adc.ch";
        case EndpointType::I2C_BUS: return "i2c.bus";
        case EndpointType::I2C_ADDR: return "i2c.addr";
        case EndpointType::SPI_BUS: return "spi.bus";
        case EndpointType::SPI_DEVICE: return "spi.device";
        case EndpointType::UART: return "uart";
        case EndpointType::ONEWIRE: return "onewire";
        default: return "unknown";
    }
}

EndpointType EndpointRegistry::parseEndpointType(const String& address) {
    if (address.startsWith("gpio.dout")) return EndpointType::GPIO_DOUT;
    if (address.startsWith("gpio.din")) return EndpointType::GPIO_DIN;
    if (address.startsWith("gpio.pwm")) return EndpointType::GPIO_PWM;
    if (address.startsWith("gpio")) return EndpointType::GPIO_PIN;
    if (address.startsWith("adc")) return EndpointType::ADC_CH;
    if (address.startsWith("i2c") && address.indexOf(':') > 0) return EndpointType::I2C_ADDR;
    if (address.startsWith("i2c")) return EndpointType::I2C_BUS;
    if (address.startsWith("spi") && address.indexOf(':') > 0) return EndpointType::SPI_DEVICE;
    if (address.startsWith("spi")) return EndpointType::SPI_BUS;
    if (address.startsWith("uart")) return EndpointType::UART;
    if (address.startsWith("onewire")) return EndpointType::ONEWIRE;
    return EndpointType::UNKNOWN;
}

} // namespace PocketOS
