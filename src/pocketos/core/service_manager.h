#ifndef POCKETOS_SERVICE_MANAGER_H
#define POCKETOS_SERVICE_MANAGER_H

#include <Arduino.h>

namespace PocketOS {

/**
 * Service Model with Deterministic Scheduler
 * 
 * Manages core services that run on deterministic intervals.
 * Services are tick-based (not time-based) for predictability.
 */

enum class ServiceState {
    STOPPED,
    RUNNING,
    PAUSED,
    FAULT
};

class Service {
public:
    virtual ~Service() {}
    
    // Service lifecycle
    virtual bool init() = 0;
    virtual void tick() = 0;
    virtual void shutdown() = 0;
    
    // Service metadata
    virtual const char* getName() const = 0;
    virtual uint32_t getTickInterval() const = 0;  // Ticks between executions
    
    ServiceState getState() const { return _state; }
    void setState(ServiceState state) { _state = state; }
    
protected:
    ServiceState _state = ServiceState::STOPPED;
};

class ServiceManager {
public:
    static void init();
    
    // Service registration
    static bool registerService(Service* service);
    static bool unregisterService(const char* name);
    
    // Service control
    static bool startService(const char* name);
    static bool stopService(const char* name);
    static bool pauseService(const char* name);
    static bool resumeService(const char* name);
    
    // Scheduler
    static void tick();  // Call from main loop
    
    // Service queries
    static int getServiceCount();
    static String getServiceList();
    static ServiceState getServiceState(const char* name);
    
private:
    static constexpr int MAX_SERVICES = 8;
    static Service* _services[MAX_SERVICES];
    static int _serviceCount;
    static uint32_t _tickCounter;
    
    static Service* findService(const char* name);
};

// Core Services

class HealthService : public Service {
public:
    bool init() override;
    void tick() override;
    void shutdown() override;
    const char* getName() const override { return "health"; }
    uint32_t getTickInterval() const override { return 1000; }  // Every 1000 ticks
    
    String getHealthReport();
};

class TelemetryService : public Service {
public:
    bool init() override;
    void tick() override;
    void shutdown() override;
    const char* getName() const override { return "telemetry"; }
    uint32_t getTickInterval() const override { return 500; }  // Every 500 ticks
    
    void recordCounter(const char* name, int value);
    void recordGauge(const char* name, int value);
    String getTelemetryReport();
};

class PersistenceService : public Service {
public:
    bool init() override;
    void tick() override;
    void shutdown() override;
    const char* getName() const override { return "persistence"; }
    uint32_t getTickInterval() const override { return 6000; }  // Every 6000 ticks (~1 min)
    
    void requestSave() { _saveRequested = true; }
    
private:
    bool _saveRequested = false;
};

} // namespace PocketOS

#endif // POCKETOS_SERVICE_MANAGER_H
