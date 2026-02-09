#include "transport_registry.h"

namespace PocketOS {

TransportRegistry& TransportRegistry::getInstance() {
    static TransportRegistry instance;
    return instance;
}

TransportRegistry::TransportRegistry() {
}

TransportRegistry::~TransportRegistry() {
}

bool TransportRegistry::registerTransport(TransportBase* transport) {
    if (!transport) {
        return false;
    }
    
    std::string name = transport->getName();
    if (transports_.find(name) != transports_.end()) {
        // Already registered
        return false;
    }
    
    transports_[name] = transport;
    return true;
}

bool TransportRegistry::unregisterTransport(const char* name) {
    if (!name) {
        return false;
    }
    
    auto it = transports_.find(name);
    if (it == transports_.end()) {
        return false;
    }
    
    transports_.erase(it);
    return true;
}

TransportBase* TransportRegistry::getTransport(const char* name) {
    if (!name) {
        return nullptr;
    }
    
    auto it = transports_.find(name);
    if (it != transports_.end()) {
        return it->second;
    }
    return nullptr;
}

TransportBase* TransportRegistry::getTransportByType(TransportBase::Type type) {
    for (auto& pair : transports_) {
        if (pair.second->getType() == type) {
            return pair.second;
        }
    }
    return nullptr;
}

std::vector<TransportBase*> TransportRegistry::getAllTransports() {
    std::vector<TransportBase*> result;
    for (auto& pair : transports_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<TransportBase*> TransportRegistry::getTransportsByTier(TransportBase::Tier tier) {
    std::vector<TransportBase*> result;
    for (auto& pair : transports_) {
        if (pair.second->getTier() == tier) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<TransportBase*> TransportRegistry::getReadyTransports() {
    std::vector<TransportBase*> result;
    for (auto& pair : transports_) {
        if (pair.second->isReady()) {
            result.push_back(pair.second);
        }
    }
    return result;
}

size_t TransportRegistry::getTransportCount() const {
    return transports_.size();
}

size_t TransportRegistry::getTransportCount(TransportBase::Tier tier) const {
    size_t count = 0;
    for (auto& pair : transports_) {
        if (pair.second->getTier() == tier) {
            count++;
        }
    }
    return count;
}

size_t TransportRegistry::getReadyCount() const {
    size_t count = 0;
    for (auto& pair : transports_) {
        if (pair.second->isReady()) {
            count++;
        }
    }
    return count;
}

bool TransportRegistry::initAll() {
    bool allSuccess = true;
    for (auto& pair : transports_) {
        if (pair.second->isSupported() && !pair.second->isReady()) {
            if (!pair.second->init()) {
                allSuccess = false;
            }
        }
    }
    return allSuccess;
}

bool TransportRegistry::initTier(TransportBase::Tier tier) {
    bool allSuccess = true;
    for (auto& pair : transports_) {
        if (pair.second->getTier() == tier && 
            pair.second->isSupported() && 
            !pair.second->isReady()) {
            if (!pair.second->init()) {
                allSuccess = false;
            }
        }
    }
    return allSuccess;
}

} // namespace PocketOS
