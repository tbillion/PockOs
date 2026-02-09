# Protocol Layers — PocketOS Protocol Layer Taxonomy

## Purpose
This document defines the **protocol layer taxonomy** in PocketOS. Protocol layers provide semantic structure and higher-level communication patterns over transports or virtual transports.

---

## Core Definition: What is a Protocol Layer?

A **protocol layer** is:
- A **software abstraction** that implements communication semantics
- Built **on top of** transports or virtual transports
- Provides **application-level functionality** (messaging, network protocols, industrial protocols)
- Does **NOT** directly interact with hardware (hardware is abstracted by transports/drivers)

**Key Characteristics:**
- Protocols **consume** transports or virtual transports
- Protocols provide **semantic meaning** to byte streams
- Protocols are **transport-agnostic** (e.g., Modbus RTU can run over RS485 or UART)

---

## Protocol vs. Transport vs. Driver

| Layer | Purpose | Examples | Built On |
|-------|---------|----------|----------|
| **Transport** | Byte/signal plumbing | I2C, SPI, UART, RS485, CAN, WiFi | Hardware peripherals |
| **Driver** | Chip/module abstraction | MCP2515, nRF24L01, SX127x, BME280 | Transports (publish virtual transports) |
| **Protocol** | Communication semantics | LoRaWAN, Modbus RTU, CANopen, MQTT | Transports or virtual transports |

**Example Stack:**
```
[Application Code]
       ↓
[LoRaWAN Protocol] ← Protocol layer (network/routing logic)
       ↓
[lora_phy0] ← Virtual transport (published by SX127x driver)
       ↓
[SX127x Driver] ← Device driver (chip abstraction)
       ↓
[spi0] ← Physical transport (SPI bus)
       ↓
[ESP32 SPI Peripheral] ← Hardware
```

---

## Protocol Categories

### 1. Network Protocols

#### LoRaWAN
**Purpose:** Long-range wide-area network protocol for IoT  
**Built On:** `lora_phy0` (virtual transport published by SX127x driver)  
**Status:** RESERVED (not yet implemented)

**Features:**
- Class A/B/C device modes
- Join procedures (OTAA, ABP)
- Adaptive data rate (ADR)
- Encryption (AES-128)
- Regional parameters (US915, EU868, AS923, etc.)

**Usage Example:**
```
> protocol bind lorawan_0 proto_lorawan lora_phy0
> protocol config lorawan_0 class A
> protocol config lorawan_0 deveui 0x0000000000000001
> protocol config lorawan_0 appeui 0x0000000000000001
> protocol config lorawan_0 appkey 0x00000000000000000000000000000001
> protocol join lorawan_0 otaa
```

**Note:** LoRaWAN is OUT OF SCOPE for this phase — taxonomy reservation only.

---

#### MQTT (Message Queuing Telemetry Transport)
**Purpose:** Lightweight publish/subscribe messaging protocol  
**Built On:** `wifi0` or `eth0` (TCP/IP required)  
**Status:** RESERVED (not yet implemented)

**Features:**
- QoS levels (0, 1, 2)
- Retained messages
- Last Will and Testament (LWT)
- TLS/SSL support

**Usage Example:**
```
> protocol bind mqtt_0 proto_mqtt wifi0
> protocol config mqtt_0 broker mqtt.example.com:1883
> protocol config mqtt_0 client_id pocketos_001
> protocol publish mqtt_0 sensors/temperature 23.5
```

---

### 2. Industrial Protocols

#### Modbus RTU
**Purpose:** Serial communication protocol for industrial devices  
**Built On:** `rs485_0` or `uart1` (serial transport)  
**Status:** RESERVED (not yet implemented)

**Features:**
- Master/slave architecture
- Function codes (read/write coils, holding registers, input registers)
- CRC error checking
- Broadcast support

**Usage Example:**
```
> protocol bind modbus_0 proto_modbus_rtu rs485_0
> protocol config modbus_0 slave_id 1
> protocol config modbus_0 baudrate 9600
> protocol read modbus_0 holding_register 100
42
```

---

#### CANopen
**Purpose:** Application layer protocol for CAN bus  
**Built On:** `can0` (CAN transport, physical or virtual)  
**Status:** RESERVED (not yet implemented)

**Features:**
- Object dictionary (OD)
- Service Data Objects (SDO)
- Process Data Objects (PDO)
- Network Management (NMT)
- Emergency messages (EMCY)

**Usage Example:**
```
> protocol bind canopen_0 proto_canopen can0
> protocol config canopen_0 node_id 5
> protocol config canopen_0 bitrate 250000
> protocol sdo_read canopen_0 0x1000 0x00
0x00000191
```

---

### 3. Application Protocols

#### HTTP/HTTPS
**Purpose:** Hypertext transfer protocol (web requests)  
**Built On:** `wifi0` or `eth0` (TCP/IP required)  
**Status:** RESERVED (not yet implemented)

**Features:**
- RESTful API support
- GET/POST/PUT/DELETE methods
- JSON/XML payloads
- TLS/SSL support

**Usage Example:**
```
> protocol bind http_0 proto_http wifi0
> protocol get http_0 https://api.example.com/sensors/1
{"id": 1, "temperature": 23.5, "humidity": 45.2}
```

---

#### CoAP (Constrained Application Protocol)
**Purpose:** Lightweight web protocol for IoT  
**Built On:** `wifi0` or `eth0` (UDP required)  
**Status:** RESERVED (not yet implemented)

**Features:**
- RESTful semantics (GET/POST/PUT/DELETE)
- Observe pattern (pub/sub)
- Blockwise transfers
- DTLS security

---

### 4. Data Serialization Protocols

#### JSON
**Purpose:** JavaScript Object Notation (text-based data format)  
**Built On:** Any byte stream (UART, WiFi, BLE, etc.)  
**Status:** RESERVED (not yet implemented)

**Usage Example:**
```
> protocol bind json_0 proto_json uart1
> protocol encode json_0 temperature:23.5,humidity:45.2
{"temperature":23.5,"humidity":45.2}
```

---

#### Protocol Buffers
**Purpose:** Binary serialization format (Google Protocol Buffers)  
**Built On:** Any byte stream  
**Status:** RESERVED (not yet implemented)

---

#### MessagePack
**Purpose:** Efficient binary serialization format  
**Built On:** Any byte stream  
**Status:** RESERVED (not yet implemented)

---

### 5. GPS/GNSS Protocols

#### NMEA 0183
**Purpose:** GPS/GNSS data format (ASCII sentences)  
**Built On:** `uart1` (GPS module UART output)  
**Status:** RESERVED (not yet implemented)

**Features:**
- GGA (position fix)
- RMC (recommended minimum data)
- GSA (satellite status)
- GSV (satellites in view)

**Usage Example:**
```
> protocol bind nmea_0 proto_nmea uart1
> protocol read nmea_0 position
Lat: 37.7749°N, Lon: 122.4194°W, Alt: 15.0m
```

---

## Protocol Lifecycle

Protocols follow a similar lifecycle to drivers:

```
UNBOUND → (bind) → BOUND → (connect) → CONNECTED
                              ↓ (error)
                            ERROR
                              ↓ (reset)
                            BOUND
```

**States:**
- `unbound` — Protocol not bound to any transport
- `bound` — Protocol bound to transport, not yet connected
- `connected` — Protocol connected and operational
- `error` — Protocol encountered error

---

## Protocol Binding Syntax

**Format:** `<transport_name>`

**Examples:**
- `lora_phy0` — LoRa physical layer (virtual transport)
- `can0` — CAN bus (physical or virtual)
- `rs485_0` — RS485 bus
- `wifi0` — WiFi interface
- `uart1` — UART port

**Note:** Unlike drivers, protocols bind to transport names directly (no extra parameters like `cs=5` or `addr=0x76`).

---

## Protocol Configuration

Protocols are configured via key-value parameters:

```
> protocol config <protocol_instance> <key> <value>
```

**Examples:**
```
> protocol config lorawan_0 class A
> protocol config modbus_0 slave_id 1
> protocol config mqtt_0 broker mqtt.example.com:1883
```

---

## Protocol Operations

Protocols expose application-level operations:

**LoRaWAN:**
```
> protocol join lorawan_0 otaa
> protocol send lorawan_0 port=1 payload=0xDEADBEEF
> protocol receive lorawan_0
```

**Modbus RTU:**
```
> protocol read modbus_0 holding_register 100
> protocol write modbus_0 coil 50 1
```

**CANopen:**
```
> protocol sdo_read canopen_0 0x1000 0x00
> protocol sdo_write canopen_0 0x1018 0x01 0x12345678
```

**MQTT:**
```
> protocol subscribe mqtt_0 sensors/temperature
> protocol publish mqtt_0 sensors/temperature 23.5
```

---

## Protocol vs. Virtual Transport: When to Use Which?

### Use a Virtual Transport when:
- The abstraction is a **communication pipe** (byte stream, message queue, radio link)
- Other code needs to **bind to it** as if it were hardware
- It provides **no application semantics** (just moves bytes/frames)

**Examples:**
- `can0` (virtual CAN bus, published by MCP2515 driver)
- `nrf24_0` (virtual 2.4GHz link, published by nRF24L01 driver)
- `lora_phy0` (virtual LoRa PHY, published by SX127x driver)

### Use a Protocol when:
- The abstraction implements **application semantics** (network layer, industrial protocol, data format)
- It provides **high-level operations** (join, read, write, subscribe, publish)
- It **interprets** the byte stream (parses messages, enforces state machines)

**Examples:**
- LoRaWAN (network protocol over `lora_phy0`)
- Modbus RTU (industrial protocol over `rs485_0`)
- MQTT (pub/sub protocol over `wifi0`)

---

## Out of Scope for This Phase

**All protocol implementations are OUT OF SCOPE for this phase.**

This document serves as a **taxonomy reservation** to:
1. Clarify the distinction between transports, drivers, and protocols
2. Reserve namespace for future protocol implementations
3. Prevent misclassification of protocols as transports (e.g., LoRaWAN)

**Future Work:**
- Implement protocol interface (`IProtocol`)
- Add protocol registry and binding logic
- Implement individual protocol stacks (LoRaWAN, Modbus RTU, etc.)
- Add CLI commands: `protocol bind/unbind/config/list`

---

## Summary

| Aspect | Description |
|--------|-------------|
| **Purpose** | Application-level communication semantics |
| **Binding** | Consumes transports or virtual transports |
| **Examples** | LoRaWAN, Modbus RTU, CANopen, MQTT, NMEA |
| **Status** | RESERVED (taxonomy only, no implementation yet) |
| **Scope** | OUT OF SCOPE for this phase |

**Remember:**
- Protocols are **NOT** transports (they provide semantics, not plumbing)
- Protocols are **NOT** drivers (they don't interact with hardware directly)
- Protocols **consume** transports/virtual transports (they require plumbing)

---

**Last Updated:** 2026-02-09  
**See Also:**
- `docs/TRANSPORT_TIERS.md` — Transport layer taxonomy
- `docs/DRIVER_CATALOG.md` — Device driver taxonomy
- `src/pocketos/core/protocol_interface.h` — IProtocol interface (future)
