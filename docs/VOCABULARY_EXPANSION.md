# PocketOS Vocabulary Expansion Plan (PART C)

## Overview

This document defines expansions to the three core vocabularies that make PocketOS truly intent-driven and extensible, per problem statement PART C.

---

## Current State Analysis

### 1. Capability Vocabulary (capability_schema.h)
**Current:**
- ParamType: BOOL, INT, FLOAT, ENUM, STRING (5 types)
- SchemaParam: name, type, RW flag, units, min/max/step
- SchemaSignal: name, type, RW flag, units
- SchemaCommand: name, argsSchema
- Limits: 8 settings, 8 signals, 8 commands per device

**Strengths:**
- Clean type system
- Constraints support (min/max/step)
- Units for physical quantities
- Line-oriented serialization

**Gaps:**
- No array/vector types for multi-value signals
- No time-series or buffered data types
- No enum value definitions
- No metadata for command arguments
- No validation rules beyond min/max

### 2. Transport/Bus Vocabulary (endpoint_registry.h)
**Current:**
- EndpointType: GPIO_PIN, GPIO_DIN, GPIO_DOUT, GPIO_PWM, ADC_CH, I2C_BUS, I2C_ADDR, SPI_BUS, UART (9 types)
- Endpoint: address string, type, resourceId
- Probing support for I2C

**Strengths:**
- Typed endpoints
- String addressing scheme
- Resource ID mapping

**Gaps:**
- No SPI device addressing
- No UART configuration (baud, parity, etc.)
- No CAN bus support
- No 1-Wire support
- No endpoint capabilities metadata
- No bus configuration parameters

### 3. Control Vocabulary (intent_api.h)
**Current:**
- Intent API v1.0.0
- 17 intent opcodes
- 7 error codes
- Line-oriented request/response

**Strengths:**
- Stable version
- Comprehensive error model
- Clean dispatcher architecture

**Gaps:**
- No async/callback intents
- No bulk operations
- No transaction support
- No intent composition
- No event subscription

---

## Expansion Proposals

### EXPANSION 1: Enhanced Capability Vocabulary

#### 1.1 Additional Parameter Types
```cpp
enum class ParamType {
    // Current
    BOOL,
    INT,
    FLOAT,
    ENUM,
    STRING,
    
    // NEW: Collection types
    INT_ARRAY,      // Fixed-size integer array
    FLOAT_ARRAY,    // Fixed-size float array
    STRING_ARRAY,   // Fixed-size string array
    
    // NEW: Special types
    TIMESTAMP,      // Unix timestamp (milliseconds)
    DURATION,       // Time duration (milliseconds)
    BYTES,          // Raw byte buffer
    
    // NEW: Structured types
    JSON_OBJECT,    // Nested structure (optional)
    KEY_VALUE       // Simple KV pairs
};
```

#### 1.2 Enum Value Definitions
```cpp
struct EnumValue {
    String name;
    int value;
    String description;
};

struct SchemaParam {
    // Existing fields...
    
    // NEW: For ENUM type
    EnumValue* enumValues;
    int enumCount;
    
    // NEW: Validation
    String validationPattern;  // Regex for STRING type
    bool required;             // Is this parameter required?
};
```

#### 1.3 Command Argument Schema
```cpp
struct CommandArg {
    String name;
    ParamType type;
    bool required;
    String description;
    float minValue;
    float maxValue;
};

struct SchemaCommand {
    bool active;
    String name;
    String description;
    
    // NEW: Structured args
    CommandArg args[4];  // Up to 4 arguments
    int argCount;
    
    // Legacy compatibility
    String argsSchema;  // Keep for simple commands
};
```

#### 1.4 Signal Buffering
```cpp
struct SchemaSignal {
    // Existing fields...
    
    // NEW: Buffering
    bool buffered;        // Does this signal buffer values?
    int bufferSize;       // Buffer capacity
    int sampleRateMs;     // Sampling period (if applicable)
};
```

**Impact:** Enables richer device descriptions, validation, and time-series data.

---

### EXPANSION 2: Enhanced Transport/Bus Vocabulary

#### 2.1 Additional Endpoint Types
```cpp
enum class EndpointType {
    // Current
    GPIO_PIN, GPIO_DIN, GPIO_DOUT, GPIO_PWM,
    ADC_CH,
    I2C_BUS, I2C_ADDR,
    SPI_BUS, UART,
    
    // NEW: Additional bus types
    SPI_DEVICE,     // SPI device with CS pin
    CAN_BUS,        // CAN bus controller
    CAN_NODE,       // CAN node with ID
    ONEWIRE_BUS,    // 1-Wire bus
    ONEWIRE_DEVICE, // 1-Wire device with ROM code
    
    // NEW: Virtual endpoints
    TIMER,          // Hardware timer
    COUNTER,        // Event counter
    WATCHDOG,       // Watchdog timer
    
    // NEW: Composite endpoints
    DISPLAY,        // Display interface (SPI/I2C/parallel)
    SENSOR_HUB,     // Multi-sensor aggregator
    
    UNKNOWN
};
```

#### 2.2 Bus Configuration
```cpp
struct BusConfig {
    // I2C configuration
    int sclPin;
    int sdaPin;
    uint32_t frequency;
    bool pullups;
    
    // SPI configuration
    int mosiPin;
    int misoPin;
    int sckPin;
    int csPin;
    uint32_t spiFrequency;
    int spiMode;
    
    // UART configuration
    int txPin;
    int rxPin;
    uint32_t baudRate;
    int dataBits;
    char parity;
    int stopBits;
    
    // CAN configuration
    int canTxPin;
    int canRxPin;
    uint32_t canBitrate;
};

struct Endpoint {
    // Existing fields...
    
    // NEW: Configuration
    BusConfig config;
    bool configured;
    
    // NEW: Capabilities
    String capabilities;  // e.g., "read,write,probe"
};
```

#### 2.3 SPI Device Addressing
```cpp
// SPI devices addressed as: "spi0:cs2" (SPI bus 0, CS pin 2)
// Parsing handled in EndpointRegistry::parseEndpointType()
```

#### 2.4 CAN Bus Addressing
```cpp
// CAN nodes addressed as: "can0:0x123" (CAN bus 0, node ID 0x123)
// Supports standard and extended IDs
```

**Impact:** Supports more bus types, device addressing, and configuration.

---

### EXPANSION 3: Enhanced Control Vocabulary

#### 3.1 Async Intent Support
```cpp
enum class IntentMode {
    SYNC,      // Block until complete (current behavior)
    ASYNC,     // Return immediately, callback on complete
    DEFERRED   // Queue for later execution
};

struct IntentRequest {
    String intent;
    String args[8];
    int argCount;
    
    // NEW: Async support
    IntentMode mode;
    String callbackId;  // For async callbacks
    int timeoutMs;      // Timeout for operation
};
```

#### 3.2 Bulk Operations
```cpp
// NEW intents:
// - "dev.bind.bulk" - Bind multiple devices in one call
// - "param.set.bulk" - Set multiple parameters atomically
// - "ep.probe.bulk" - Probe multiple endpoints

struct IntentRequest {
    // ...existing...
    
    // NEW: Bulk operations
    bool bulk;
    int bulkCount;
};
```

#### 3.3 Event Subscription
```cpp
// NEW intents:
// - "event.subscribe" - Subscribe to device events
// - "event.unsubscribe" - Unsubscribe
// - "event.list" - List subscriptions

enum class EventType {
    PARAM_CHANGED,    // Parameter value changed
    STATE_CHANGED,    // Device state changed
    SIGNAL_UPDATED,   // Signal value updated
    ERROR_OCCURRED,   // Error event
    THRESHOLD_CROSSED // Value crossed threshold
};
```

#### 3.4 Transaction Support
```cpp
// NEW intents:
// - "txn.begin" - Start transaction
// - "txn.commit" - Commit transaction
// - "txn.rollback" - Rollback transaction

// Allows atomic multi-operation sequences
```

#### 3.5 Intent Composition
```cpp
// NEW intent:
// - "macro.exec" - Execute macro (sequence of intents)

struct MacroStep {
    String intent;
    String args[8];
    int argCount;
    String condition;  // Optional: execute if condition true
};
```

**Impact:** Enables async operations, events, transactions, and macros.

---

## Implementation Priority

### Phase 1: High-Value Extensions (Implement First)
1. **Enum value definitions** - Improves validation and UI generation
2. **Command argument schema** - Better command documentation
3. **SPI device addressing** - Common bus type support
4. **CAN bus support** - Industrial/automotive use cases

### Phase 2: Medium-Value Extensions
5. **Array types** - Multi-value signals
6. **Bus configuration** - Explicit configuration
7. **Timestamp/Duration types** - Time-based operations
8. **Event subscription** - Reactive operations

### Phase 3: Advanced Extensions
9. **Async intents** - Non-blocking operations
10. **Transaction support** - Atomic operations
11. **Bulk operations** - Performance optimization
12. **Macro system** - Complex workflows

---

## Backward Compatibility

All expansions maintain backward compatibility:
- Existing enums extended, not replaced
- New fields added with defaults
- Old intent opcodes unchanged
- Version remains v1.0.0 (or bump to v1.1.0 if needed)

---

## Testing Strategy

For each expansion:
1. Add schema test cases
2. Verify serialization format
3. Test backward compatibility
4. Document in SMOKE_TEST.md
5. Update REQUIREMENTS_VERIFICATION.md

---

## Documentation Updates Required

1. **capability_schema.h** - Add new types, structs
2. **endpoint_registry.h** - Add new endpoint types
3. **intent_api.h** - Add new intents
4. **README.md** - Document new capabilities
5. **SMOKE_TEST.md** - Add new test cases
6. **VOCABULARY.md** - NEW: Complete vocabulary reference

---

## Next Steps

1. Create VOCABULARY.md reference document
2. Implement Phase 1 extensions
3. Add test cases for each extension
4. Update documentation
5. Verify backward compatibility

---

**Status:** Plan complete, ready for implementation.
**Recommendation:** Start with Phase 1 (enum definitions, command args, SPI/CAN support).
