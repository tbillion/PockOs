#include "transport_base.h"

namespace PocketOS {

TransportBase::TransportBase(const char* name, Type type, Tier tier)
    : name_(name)
    , type_(type)
    , tier_(tier)
    , state_(State::UNINITIALIZED)
    , initTime_(0)
    , errorCount_(0)
    , successCount_(0)
    , lastErrorTime_(0)
{
}

TransportBase::~TransportBase() {
}

bool TransportBase::reset() {
    if (!deinit()) {
        return false;
    }
    clearError();
    return init();
}

bool TransportBase::setConfig(const std::string& key, const std::string& value) {
    config_[key] = value;
    return true;
}

std::string TransportBase::getConfig(const std::string& key) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        return it->second;
    }
    return "";
}

std::map<std::string, std::string> TransportBase::getAllConfig() const {
    return config_;
}

TransportBase::Status TransportBase::getStatus() const {
    Status status;
    status.state = state_;
    status.lastError = lastErrorTime_;
    status.initTime = initTime_;
    status.uptime = (state_ == State::READY && initTime_ > 0) ? (millis() - initTime_) : 0;
    status.errorCount = errorCount_;
    status.successCount = successCount_;
    status.lastErrorMsg = lastErrorMsg_;
    return status;
}

std::string TransportBase::getCapabilities() const {
    return "base";
}

void TransportBase::setState(State newState) {
    state_ = newState;
    if (newState == State::READY && initTime_ == 0) {
        initTime_ = millis();
    }
}

void TransportBase::setError(const char* msg) {
    state_ = State::ERROR;
    errorCount_++;
    lastErrorTime_ = millis();
    lastErrorMsg_ = msg ? msg : "Unknown error";
}

void TransportBase::clearError() {
    if (state_ == State::ERROR) {
        state_ = State::UNINITIALIZED;
    }
    lastErrorMsg_ = "";
}

void TransportBase::incrementSuccess() {
    successCount_++;
}

const char* TransportBase::tierToString(Tier tier) {
    switch (tier) {
        case Tier::TIER0: return "TIER0";
        case Tier::TIER1: return "TIER1";
        case Tier::TIER2: return "TIER2";
        default: return "UNKNOWN";
    }
}

const char* TransportBase::stateToString(State state) {
    switch (state) {
        case State::UNINITIALIZED: return "UNINITIALIZED";
        case State::INITIALIZING: return "INITIALIZING";
        case State::READY: return "READY";
        case State::ERROR: return "ERROR";
        case State::DISABLED: return "DISABLED";
        default: return "UNKNOWN";
    }
}

const char* TransportBase::typeToString(Type type) {
    switch (type) {
        // Tier 0
        case Type::GPIO: return "GPIO";
        case Type::ADC: return "ADC";
        case Type::PWM: return "PWM";
        case Type::I2C: return "I2C";
        case Type::SPI: return "SPI";
        case Type::UART: return "UART";
        case Type::ONEWIRE: return "ONEWIRE";
        // Tier 1
        case Type::RS485: return "RS485";
        case Type::RS232: return "RS232";
        case Type::CAN: return "CAN";
        case Type::ETHERNET: return "ETHERNET";
        case Type::MCP2515: return "MCP2515";
        case Type::NRF24L01: return "NRF24L01";
        case Type::LORAWAN: return "LORAWAN";
        case Type::RC_TRANSCEIVER: return "RC_TRANSCEIVER";
        // Tier 2
        case Type::WIFI: return "WIFI";
        case Type::BLE: return "BLE";
        default: return "UNKNOWN";
    }
}

} // namespace PocketOS
