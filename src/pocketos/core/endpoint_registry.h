#ifndef POCKETOS_ENDPOINT_REGISTRY_H
#define POCKETOS_ENDPOINT_REGISTRY_H

#include <Arduino.h>

namespace PocketOS {

#define MAX_ENDPOINTS 32

enum class EndpointType {
    GPIO_PIN,
    GPIO_DIN,
    GPIO_DOUT,
    GPIO_PWM,
    ADC_CH,
    I2C_BUS,
    I2C_ADDR,
    SPI_BUS,
    UART,
    UNKNOWN
};

struct Endpoint {
    bool active;
    String address;  // e.g., "gpio.pin.2", "i2c0:0x48", "adc.ch.0"
    EndpointType type;
    int resourceId;  // Physical resource ID (pin number, channel, etc.)
    
    Endpoint() : active(false), address(""), type(EndpointType::UNKNOWN), resourceId(-1) {}
};

class EndpointRegistry {
public:
    static void init();
    
    // Register/unregister endpoints
    static bool registerEndpoint(const String& address, EndpointType type, int resourceId);
    static bool unregisterEndpoint(const String& address);
    
    // Query endpoints
    static bool endpointExists(const String& address);
    static EndpointType getEndpointType(const String& address);
    static int getEndpointResource(const String& address);
    
    // List endpoints
    static String listEndpoints();
    
    // Probe endpoint (e.g., I2C bus scan)
    static String probeEndpoint(const String& address);
    
    // Auto-register available endpoints at init
    static void autoRegisterEndpoints();
    
private:
    static Endpoint endpoints[MAX_ENDPOINTS];
    static int endpointCount;
    
    static int findEndpoint(const String& address);
    static int findFreeSlot();
    static const char* endpointTypeToString(EndpointType type);
    static EndpointType parseEndpointType(const String& address);
};

} // namespace PocketOS

#endif // POCKETOS_ENDPOINT_REGISTRY_H
