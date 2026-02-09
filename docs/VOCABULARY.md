# PocketOS Vocabulary Reference

## Overview

PocketOS is built on three core vocabularies that enable intent-driven, extensible embedded systems development. This document serves as the complete reference for all three vocabularies.

---

## 1. Capability Vocabulary

The capability vocabulary defines how devices describe their features, parameters, and operations.

### 1.1 Parameter Types

| Type | Description | Example Use | Constraints |
|------|-------------|-------------|-------------|
| `BOOL` | Boolean value | On/off states, flags | true/false |
| `INT` | Integer value | Counts, IDs, discrete values | min/max/step |
| `FLOAT` | Floating-point | Measurements, analog values | min/max/step |
| `ENUM` | Enumerated | Mode selection, states | Predefined values |
| `STRING` | Text string | Names, messages, config | Max length |

### 1.2 Schema Components

#### Settings
Configurable parameters that control device behavior.

**Properties:**
- `name`: Parameter identifier
- `type`: ParamType enum
- `readWrite`: true (RW) or false (RO)
- `units`: Physical units (e.g., "V", "°C", "Hz")
- `minValue`: Minimum allowed value
- `maxValue`: Maximum allowed value
- `stepValue`: Increment step

**Example:**
```
brightness:int:rw:0.00-100.00:1:percent
```

#### Signals
Observable values that reflect device state or measurements.

**Properties:**
- `name`: Signal identifier
- `type`: ParamType enum
- `readWrite`: Usually false (read-only)
- `units`: Physical units

**Example:**
```
temperature:float:ro:°C
```

#### Commands
Actions that can be invoked on the device.

**Properties:**
- `name`: Command identifier
- `argsSchema`: Argument description

**Example:**
```
calibrate offset:float
```

### 1.3 Schema Serialization Format

Line-oriented, human-readable format:

```
[settings]
param_name:type:rw:min-max:step:units

[signals]
signal_name:type:rw:units

[commands]
command_name arg_schema
```

### 1.4 Proposed Extensions

**Array Types:**
- `INT_ARRAY`: Fixed-size integer arrays
- `FLOAT_ARRAY`: Fixed-size float arrays
- `STRING_ARRAY`: Fixed-size string arrays

**Time Types:**
- `TIMESTAMP`: Unix timestamp (ms)
- `DURATION`: Time duration (ms)

**Structured Types:**
- `BYTES`: Raw byte buffer
- `KEY_VALUE`: Simple KV pairs

---

## 2. Transport/Bus Vocabulary

The transport vocabulary defines addressable hardware endpoints and bus types.

### 2.1 Endpoint Types

| Type | Description | Address Format | Example |
|------|-------------|----------------|---------|
| `GPIO_PIN` | Generic GPIO | gpio.pin.N | gpio.pin.2 |
| `GPIO_DIN` | Digital input | gpio.din.N | gpio.din.4 |
| `GPIO_DOUT` | Digital output | gpio.dout.N | gpio.dout.2 |
| `GPIO_PWM` | PWM output | gpio.pwm.N | gpio.pwm.5 |
| `ADC_CH` | Analog input | adc.ch.N | adc.ch.0 |
| `I2C_BUS` | I2C bus | i2cN | i2c0 |
| `I2C_ADDR` | I2C device | i2cN:0xXX | i2c0:0x48 |
| `SPI_BUS` | SPI bus | spiN | spi0 |
| `UART` | UART port | uartN | uart1 |

### 2.2 Addressing Scheme

#### GPIO Endpoints
```
gpio.<function>.<pin_number>
```
- Function: pin, din, dout, pwm
- Pin number: Physical GPIO number

#### ADC Endpoints
```
adc.ch.<channel_number>
```
- Channel number: ADC channel (0-based)

#### I2C Endpoints
```
i2c<bus_number>              # Bus endpoint
i2c<bus_number>:0x<address>  # Device endpoint
```
- Bus number: I2C controller (0-based)
- Address: 7-bit I2C address (hex)

#### SPI Endpoints
```
spi<bus_number>              # Bus endpoint
spi<bus_number>:cs<pin>      # Device endpoint (proposed)
```

#### UART Endpoints
```
uart<port_number>
```
- Port number: UART controller (0-based)

### 2.3 Endpoint Operations

| Operation | Description | Applicable To |
|-----------|-------------|---------------|
| `list` | List all endpoints | All |
| `probe` | Scan for devices | I2C, SPI (proposed) |
| `register` | Add endpoint | All |
| `unregister` | Remove endpoint | All |

### 2.4 Proposed Extensions

**New Endpoint Types:**
- `SPI_DEVICE`: SPI device with CS pin
- `CAN_BUS`: CAN bus controller
- `CAN_NODE`: CAN node with ID (can0:0x123)
- `ONEWIRE_BUS`: 1-Wire bus
- `ONEWIRE_DEVICE`: 1-Wire device with ROM

**Bus Configuration:**
- I2C: frequency, pullups, pins
- SPI: mode, frequency, pins
- UART: baud, parity, stop bits, pins
- CAN: bitrate, pins

---

## 3. Control Vocabulary

The control vocabulary defines intents (operations) and responses.

### 3.1 Intent Opcodes

#### System Information
| Intent | Args | Description |
|--------|------|-------------|
| `sys.info` | - | System information (board, chip, heap) |
| `hal.caps` | - | Hardware capabilities (GPIO, ADC, etc.) |

#### Endpoint Operations
| Intent | Args | Description |
|--------|------|-------------|
| `ep.list` | - | List all endpoints |
| `ep.probe` | endpoint | Probe endpoint (e.g., I2C scan) |

#### Device Operations
| Intent | Args | Description |
|--------|------|-------------|
| `dev.list` | - | List bound devices |
| `dev.bind` | driver, endpoint | Bind driver to endpoint |
| `dev.unbind` | device_id | Unbind device |
| `dev.enable` | device_id | Enable device |
| `dev.disable` | device_id | Disable device |

#### Parameter Operations
| Intent | Args | Description |
|--------|------|-------------|
| `param.get` | device_id, param | Get parameter value |
| `param.set` | device_id, param, value | Set parameter value |

#### Schema Operations
| Intent | Args | Description |
|--------|------|-------------|
| `schema.get` | device_id | Get device schema |

#### Logging Operations
| Intent | Args | Description |
|--------|------|-------------|
| `log.tail` | [count] | Show recent log entries |
| `log.clear` | - | Clear log buffer |

#### Persistence Operations
| Intent | Args | Description |
|--------|------|-------------|
| `persist.save` | - | Save configuration to NVS |
| `persist.load` | - | Load configuration from NVS |

### 3.2 Error Codes

| Code | Value | Description | When Used |
|------|-------|-------------|-----------|
| `OK` | 0 | Success | Operation completed successfully |
| `ERR_BAD_ARGS` | 1 | Bad arguments | Invalid or missing arguments |
| `ERR_NOT_FOUND` | 2 | Not found | Resource not found (device, endpoint) |
| `ERR_CONFLICT` | 3 | Conflict | Resource already in use |
| `ERR_IO` | 4 | I/O error | Hardware communication failure |
| `ERR_UNSUPPORTED` | 5 | Unsupported | Operation not supported |
| `ERR_INTERNAL` | 6 | Internal error | Unexpected internal failure |

### 3.3 Response Format

Line-oriented format with status and data:

**Success Response:**
```
<status_code>
<data_line_1>
<data_line_2>
...
```

**Error Response:**
```
<error_code>
<error_message>
```

**Examples:**
```
# sys.info response
version=1.0.0
board=ESP32
chip=ESP32
flash_size=4194304

# Error response
ERR_NOT_FOUND
Device not found
```

### 3.4 Proposed Extensions

**Async Operations:**
- Intent mode: SYNC, ASYNC, DEFERRED
- Callback mechanism for async completion

**Bulk Operations:**
- `dev.bind.bulk`: Bind multiple devices
- `param.set.bulk`: Set multiple parameters atomically

**Event Subscription:**
- `event.subscribe`: Subscribe to device events
- `event.unsubscribe`: Unsubscribe from events
- `event.list`: List active subscriptions

**Transaction Support:**
- `txn.begin`: Start transaction
- `txn.commit`: Commit transaction
- `txn.rollback`: Rollback transaction

**Macro System:**
- `macro.define`: Define macro (sequence of intents)
- `macro.exec`: Execute macro
- `macro.list`: List defined macros

---

## 4. Vocabulary Versioning

Current version: **v1.0.0**

### Versioning Policy

- **Major version**: Breaking changes to vocabulary
- **Minor version**: Backward-compatible additions
- **Patch version**: Bug fixes, clarifications

### Compatibility

- Code must check `INTENT_API_VERSION`
- Clients should handle unknown types gracefully
- New types added as enum extensions
- Deprecated types marked but not removed

---

## 5. Usage Examples

### Example 1: GPIO Output Control

```
# Bind GPIO driver
> bind gpio.dout gpio.dout.2
device_id=1

# Get schema
> schema 1
[settings]
state:bool:rw:0.00-1.00
pin:int:ro

# Set state
> param set 1 state 1
OK

# Read state
> param get 1 state
state=1
```

### Example 2: I2C Device Discovery

```
# List endpoints
> ep list
i2c0 (i2c.bus) [0]

# Probe I2C bus
> ep probe i2c0
I2C0 scan:
  0x48
  0x76

# Discovered devices can be bound
> bind temp.sensor i2c0:0x48
device_id=2
```

### Example 3: Multi-Device System

```
# Bind multiple devices
> bind gpio.dout gpio.dout.2
device_id=1
> bind adc.in adc.ch.0
device_id=2
> bind pwm.out gpio.pwm.5
device_id=3

# List all devices
> dev list
dev1: gpio.dout @ gpio.dout.2 [READY]
dev2: adc.in @ adc.ch.0 [READY]
dev3: pwm.out @ gpio.pwm.5 [READY]

# Save configuration
> persist save
OK
```

---

## 6. Extension Guidelines

When extending vocabularies:

1. **Maintain Backward Compatibility**
   - Add, don't replace enum values
   - New fields have sensible defaults
   - Existing intents unchanged

2. **Follow Naming Conventions**
   - Capabilities: lowercase, descriptive
   - Endpoints: hierarchical (bus.type.id)
   - Intents: verb.noun format

3. **Document Everything**
   - Update this vocabulary reference
   - Add test cases to SMOKE_TEST.md
   - Update REQUIREMENTS_VERIFICATION.md

4. **Test Thoroughly**
   - Unit tests for parsing
   - Integration tests for intents
   - Backward compatibility tests

---

## 7. Vocabulary Queries

### Query Current Capabilities
```
# Get all supported parameter types
Not yet implemented - would need schema.types intent

# Get all supported endpoint types
Not yet implemented - would need ep.types intent

# Get all supported intents
Not yet implemented - would need intent.list
```

### Future: Self-Describing System
Add intents to query vocabulary at runtime:
- `vocab.param_types` - List supported parameter types
- `vocab.endpoint_types` - List supported endpoint types
- `vocab.intents` - List supported intents
- `vocab.errors` - List error codes

---

## 8. Reference Implementation

See these files for implementation:
- **Capability**: `src/pocketos/core/capability_schema.{h,cpp}`
- **Transport**: `src/pocketos/core/endpoint_registry.{h,cpp}`
- **Control**: `src/pocketos/core/intent_api.{h,cpp}`

---

**Version:** 1.0.0  
**Last Updated:** 2026-02-08  
**Status:** Current vocabulary + proposed extensions documented
