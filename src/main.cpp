#include <Arduino.h>
#include "pocketos/core/logger.h"
#include "pocketos/core/hal.h"
#include "pocketos/core/intent_api.h"
#include "pocketos/core/resource_manager.h"
#include "pocketos/core/endpoint_registry.h"
#include "pocketos/core/device_registry.h"
#include "pocketos/core/persistence.h"
#include "pocketos/core/device_identifier.h"
#include "pocketos/cli/cli.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize core systems in order
    PocketOS::Logger::init();
    PocketOS::HAL::init();
    PocketOS::IntentAPI::init();
    PocketOS::ResourceManager::init();
    PocketOS::EndpointRegistry::init();
    PocketOS::DeviceRegistry::init();
    PocketOS::DeviceIdentifier::init();
    PocketOS::Persistence::init();
    
    // Load saved configuration
    PocketOS::Persistence::loadAll();
    
    // Initialize CLI last
    PocketOS::CLI::init();
    
    PocketOS::Logger::info("PocketOS Ready");
    Serial.print("> ");
}

void loop() {
    PocketOS::CLI::process();
    PocketOS::DeviceRegistry::updateAll();
    delay(10);
}
