#ifndef POCKETOS_TRANSPORT_REGISTRY_H
#define POCKETOS_TRANSPORT_REGISTRY_H

#include "transport_base.h"
#include <vector>
#include <map>

namespace PocketOS {

/**
 * Transport Registry
 * 
 * Singleton registry for all transport instances.
 * Provides discovery, lookup, and enumeration of transports.
 */
class TransportRegistry {
public:
    // Get singleton instance
    static TransportRegistry& getInstance();
    
    // Registration
    bool registerTransport(TransportBase* transport);
    bool unregisterTransport(const char* name);
    
    // Lookup
    TransportBase* getTransport(const char* name);
    TransportBase* getTransportByType(TransportBase::Type type);
    
    // Enumeration
    std::vector<TransportBase*> getAllTransports();
    std::vector<TransportBase*> getTransportsByTier(TransportBase::Tier tier);
    std::vector<TransportBase*> getReadyTransports();
    
    // Statistics
    size_t getTransportCount() const;
    size_t getTransportCount(TransportBase::Tier tier) const;
    size_t getReadyCount() const;
    
    // Initialization
    bool initAll();
    bool initTier(TransportBase::Tier tier);
    
private:
    TransportRegistry();
    ~TransportRegistry();
    
    // Prevent copying
    TransportRegistry(const TransportRegistry&) = delete;
    TransportRegistry& operator=(const TransportRegistry&) = delete;
    
    std::map<std::string, TransportBase*> transports_;
};

} // namespace PocketOS

#endif // POCKETOS_TRANSPORT_REGISTRY_H
