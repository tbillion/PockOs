#ifndef POCKETOS_RESOURCE_MANAGER_H
#define POCKETOS_RESOURCE_MANAGER_H

#include <Arduino.h>

namespace PocketOS {

#define MAX_RESOURCES 64

enum class ResourceType {
    GPIO_PIN,
    ADC_CHANNEL,
    PWM_CHANNEL,
    I2C_BUS,
    SPI_BUS,
    UART
};

struct ResourceClaim {
    bool claimed;
    ResourceType type;
    int resourceId;
    String ownerId;
    
    ResourceClaim() : claimed(false), type(ResourceType::GPIO_PIN), resourceId(-1), ownerId("") {}
};

class ResourceManager {
public:
    static void init();
    
    // Resource management
    static bool claim(ResourceType type, int resourceId, const String& ownerId);
    static bool release(ResourceType type, int resourceId, const String& ownerId);
    static bool isClaimed(ResourceType type, int resourceId);
    static String getOwner(ResourceType type, int resourceId);
    
    // List all claims
    static String listClaims();
    
    // Check if resource is available
    static bool isAvailable(ResourceType type, int resourceId);
    
private:
    static ResourceClaim claims[MAX_RESOURCES];
    static int claimCount;
    
    static int findClaim(ResourceType type, int resourceId);
    static int findFreeSlot();
    static const char* resourceTypeToString(ResourceType type);
};

} // namespace PocketOS

#endif // POCKETOS_RESOURCE_MANAGER_H
