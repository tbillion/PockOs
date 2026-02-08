#include "service_manager.h"
#include "logger.h"
#include "hal.h"
#include "device_registry.h"
#include "persistence.h"

namespace PocketOS {

// ServiceManager static members
Service* ServiceManager::_services[ServiceManager::MAX_SERVICES];
int ServiceManager::_serviceCount = 0;
uint32_t ServiceManager::_tickCounter = 0;

void ServiceManager::init() {
    _serviceCount = 0;
    _tickCounter = 0;
    Logger::info("ServiceManager initialized");
}

bool ServiceManager::registerService(Service* service) {
    if (_serviceCount >= MAX_SERVICES) {
        Logger::error("ServiceManager: Max services reached");
        return false;
    }
    
    _services[_serviceCount++] = service;
    Logger::info(String("Service registered: ") + service->getName());
    return true;
}

bool ServiceManager::unregisterService(const char* name) {
    for (int i = 0; i < _serviceCount; i++) {
        if (strcmp(_services[i]->getName(), name) == 0) {
            // Shift remaining services
            for (int j = i; j < _serviceCount - 1; j++) {
                _services[j] = _services[j + 1];
            }
            _serviceCount--;
            Logger::info(String("Service unregistered: ") + name);
            return true;
        }
    }
    return false;
}

bool ServiceManager::startService(const char* name) {
    Service* service = findService(name);
    if (!service) return false;
    
    if (service->getState() != ServiceState::STOPPED) {
        return false;
    }
    
    if (service->init()) {
        service->setState(ServiceState::RUNNING);
        Logger::info(String("Service started: ") + name);
        return true;
    }
    
    service->setState(ServiceState::FAULT);
    Logger::error(String("Service start failed: ") + name);
    return false;
}

bool ServiceManager::stopService(const char* name) {
    Service* service = findService(name);
    if (!service) return false;
    
    service->shutdown();
    service->setState(ServiceState::STOPPED);
    Logger::info(String("Service stopped: ") + name);
    return true;
}

bool ServiceManager::pauseService(const char* name) {
    Service* service = findService(name);
    if (!service) return false;
    
    if (service->getState() == ServiceState::RUNNING) {
        service->setState(ServiceState::PAUSED);
        return true;
    }
    return false;
}

bool ServiceManager::resumeService(const char* name) {
    Service* service = findService(name);
    if (!service) return false;
    
    if (service->getState() == ServiceState::PAUSED) {
        service->setState(ServiceState::RUNNING);
        return true;
    }
    return false;
}

void ServiceManager::tick() {
    _tickCounter++;
    
    for (int i = 0; i < _serviceCount; i++) {
        Service* service = _services[i];
        if (service->getState() == ServiceState::RUNNING) {
            if (_tickCounter % service->getTickInterval() == 0) {
                service->tick();
            }
        }
    }
}

int ServiceManager::getServiceCount() {
    return _serviceCount;
}

String ServiceManager::getServiceList() {
    String list = "";
    for (int i = 0; i < _serviceCount; i++) {
        if (i > 0) list += ", ";
        list += _services[i]->getName();
    }
    return list;
}

ServiceState ServiceManager::getServiceState(const char* name) {
    Service* service = findService(name);
    return service ? service->getState() : ServiceState::STOPPED;
}

Service* ServiceManager::findService(const char* name) {
    for (int i = 0; i < _serviceCount; i++) {
        if (strcmp(_services[i]->getName(), name) == 0) {
            return _services[i];
        }
    }
    return nullptr;
}

// HealthService implementation
bool HealthService::init() {
    return true;
}

void HealthService::tick() {
    // Perform health checks
    uint32_t freeHeap = HAL::getFreeHeap();
    int deviceCount = DeviceRegistry::getDeviceCount();
    
    // Log health metrics periodically
    static int healthCounter = 0;
    if (++healthCounter >= 10) {  // Every 10 ticks = ~10 seconds
        Logger::info("Health: heap=" + String(freeHeap) + " devices=" + String(deviceCount));
        healthCounter = 0;
    }
}

void HealthService::shutdown() {
    // Nothing to clean up
}

String HealthService::getHealthReport() {
    String report = "=== System Health Report ===\n";
    report += "Free Heap: " + String(HAL::getFreeHeap()) + " bytes\n";
    report += "Device Count: " + String(DeviceRegistry::getDeviceCount()) + "\n";
    report += "Uptime: " + String(millis() / 1000) + " seconds\n";
    return report;
}

// TelemetryService implementation
bool TelemetryService::init() {
    return true;
}

void TelemetryService::tick() {
    // Collect telemetry data
    // (In full implementation, would collect various metrics)
}

void TelemetryService::shutdown() {
    // Nothing to clean up
}

void TelemetryService::recordCounter(const char* name, int value) {
    // Would record counter metric
}

void TelemetryService::recordGauge(const char* name, int value) {
    // Would record gauge metric
}

String TelemetryService::getTelemetryReport() {
    String report = "=== Telemetry Report ===\n";
    report += "System uptime: " + String(millis() / 1000) + "s\n";
    report += "Tick count: " + String(ServiceManager::_tickCounter) + "\n";
    return report;
}

// PersistenceService implementation
bool PersistenceService::init() {
    return true;
}

void PersistenceService::tick() {
    if (_saveRequested) {
        Persistence::saveAll();
        _saveRequested = false;
    }
}

void PersistenceService::shutdown() {
    // Final save on shutdown
    Persistence::saveAll();
}

} // namespace PocketOS
