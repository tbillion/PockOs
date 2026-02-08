#include <Arduino.h>
#include "pocketos/core/logger.h"
#include "pocketos/core/hal.h"
#include "pocketos/core/intent_api.h"
#include "pocketos/core/resource_manager.h"
#include "pocketos/core/endpoint_registry.h"
#include "pocketos/core/device_registry.h"
#include "pocketos/core/persistence.h"
#include "pocketos/core/device_identifier.h"
#include "pocketos/core/pcf1_config.h"
#include "pocketos/core/service_manager.h"
#include "pocketos/platform/platform_pack.h"
#include "pocketos/cli/cli.h"

// Global service instances
PocketOS::HealthService g_healthService;
PocketOS::TelemetryService g_telemetryService;
PocketOS::PersistenceService g_persistenceService;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize platform pack first
    PocketOS::g_platformPack = PocketOS::createPlatformPack();
    if (!PocketOS::g_platformPack) {
        Serial.println("ERROR: Platform not supported");
        while(1) delay(1000);
    }
    
    Serial.println("\n===== PocketOS Universal Core v1 =====");
    Serial.print("Platform: ");
    Serial.println(PocketOS::g_platformPack->getName());
    Serial.print("Version: ");
    Serial.println(PocketOS::g_platformPack->getVersion());
    Serial.println("======================================\n");
    
    // Initialize core systems in order
    PocketOS::Logger::init();
    PocketOS::HAL::init();
    PocketOS::IntentAPI::init();
    PocketOS::ResourceManager::init();
    PocketOS::EndpointRegistry::init();
    PocketOS::DeviceRegistry::init();
    PocketOS::DeviceIdentifier::init();
    PocketOS::Persistence::init();
    PocketOS::PCF1Config::init();
    
    // Initialize service manager
    PocketOS::ServiceManager::init();
    
    // Register and start core services
    PocketOS::ServiceManager::registerService(&g_healthService);
    PocketOS::ServiceManager::registerService(&g_telemetryService);
    PocketOS::ServiceManager::registerService(&g_persistenceService);
    
    PocketOS::ServiceManager::startService("health");
    PocketOS::ServiceManager::startService("telemetry");
    PocketOS::ServiceManager::startService("persistence");
    
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
    PocketOS::ServiceManager::tick();  // Run services on deterministic schedule
    delay(10);
}
