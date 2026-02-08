#include "resource_manager.h"
#include "logger.h"

namespace PocketOS {

ResourceClaim ResourceManager::claims[MAX_RESOURCES];
int ResourceManager::claimCount = 0;

void ResourceManager::init() {
    claimCount = 0;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        claims[i].claimed = false;
    }
    Logger::info("Resource Manager initialized");
}

bool ResourceManager::claim(ResourceType type, int resourceId, const String& ownerId) {
    // Check if already claimed
    int existing = findClaim(type, resourceId);
    if (existing >= 0) {
        if (claims[existing].ownerId == ownerId) {
            // Same owner, allow re-claim
            return true;
        }
        Logger::warning("Resource already claimed by another owner");
        return false;
    }
    
    // Find free slot
    int slot = findFreeSlot();
    if (slot < 0) {
        Logger::error("No free resource slots");
        return false;
    }
    
    // Claim resource
    claims[slot].claimed = true;
    claims[slot].type = type;
    claims[slot].resourceId = resourceId;
    claims[slot].ownerId = ownerId;
    claimCount++;
    
    return true;
}

bool ResourceManager::release(ResourceType type, int resourceId, const String& ownerId) {
    int idx = findClaim(type, resourceId);
    if (idx < 0) {
        return false;
    }
    
    if (claims[idx].ownerId != ownerId) {
        Logger::warning("Cannot release resource claimed by another owner");
        return false;
    }
    
    claims[idx].claimed = false;
    claims[idx].ownerId = "";
    claimCount--;
    return true;
}

bool ResourceManager::isClaimed(ResourceType type, int resourceId) {
    return findClaim(type, resourceId) >= 0;
}

String ResourceManager::getOwner(ResourceType type, int resourceId) {
    int idx = findClaim(type, resourceId);
    if (idx >= 0) {
        return claims[idx].ownerId;
    }
    return "";
}

String ResourceManager::listClaims() {
    String result = "";
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (claims[i].claimed) {
            result += resourceTypeToString(claims[i].type);
            result += ":" + String(claims[i].resourceId);
            result += " -> " + claims[i].ownerId + "\n";
        }
    }
    if (result.length() == 0) {
        result = "No resources claimed\n";
    }
    return result;
}

bool ResourceManager::isAvailable(ResourceType type, int resourceId) {
    return !isClaimed(type, resourceId);
}

int ResourceManager::findClaim(ResourceType type, int resourceId) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (claims[i].claimed && 
            claims[i].type == type && 
            claims[i].resourceId == resourceId) {
            return i;
        }
    }
    return -1;
}

int ResourceManager::findFreeSlot() {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!claims[i].claimed) {
            return i;
        }
    }
    return -1;
}

const char* ResourceManager::resourceTypeToString(ResourceType type) {
    switch (type) {
        case ResourceType::GPIO_PIN: return "gpio";
        case ResourceType::ADC_CHANNEL: return "adc";
        case ResourceType::PWM_CHANNEL: return "pwm";
        case ResourceType::I2C_BUS: return "i2c";
        case ResourceType::SPI_BUS: return "spi";
        case ResourceType::UART: return "uart";
        default: return "unknown";
    }
}

} // namespace PocketOS
