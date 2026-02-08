#include "resources.h"
#include "logger.h"

namespace PocketOS {

void Resources::init() {
    Logger::info("Resources initialized");
}

void Resources::release() {
    Logger::info("Resources released");
}

bool Resources::allocate(const char* resourceId) {
    Logger::debug("Resource allocated");
    return true;
}

bool Resources::deallocate(const char* resourceId) {
    Logger::debug("Resource deallocated");
    return true;
}

} // namespace PocketOS
