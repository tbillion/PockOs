# Transport Tiers — PocketOS Transport Layer Taxonomy

## Purpose
This document defines the **transport tier classification** in PocketOS. Transports are **plumbing-level abstractions** that provide byte/signal pipes, resource ownership, configuration, and status reporting.

## Core Definition: What is a Transport?

A **transport** is:
- A **physical or logical communication layer** that moves bytes or signals
- Manages **hardware resources** (pins, buses, peripherals)
- Provides **configuration** and **status reporting**
- Implements the `TransportBase` interface

**Important:** Transports are **NOT**:
- Device chips (e.g., MCP2515, nRF24L01, SX127x are device drivers, not transports)
- Protocol stacks (e.g., LoRaWAN, Modbus RTU, CANopen are protocols, not transports)
- Application-layer abstractions

---

## Tier 0: Basic Hardware Transports

**Definition:** Direct MCU peripheral access. Single instance per physical bus/channel.

**Examples:**
- **GPIO** — General purpose I/O pins (digital/analog read/write)
- **ADC** — Analog-to-digital conversion channels
- **PWM** — Pulse-width modulation outputs
- **I2C** — Inter-Integrated Circuit bus (master mode)
- **SPI** — Serial Peripheral Interface bus
- **UART** — Universal Asynchronous Receiver/Transmitter (serial)
- **OneWire** — 1-Wire bus protocol

**Characteristics:**
- Direct hardware mapping
- No protocol overhead
- Platform-specific (ESP32, ESP8266, RP2040)
- Foundation for all higher-level abstractions

**Example Endpoint Names:**
- `gpio.pin.5` (GPIO pin 5)
- `adc.ch.0` (ADC channel 0)
- `i2c0` (I2C bus 0)
- `spi0` (SPI bus 0)
- `uart1` (UART port 1)

---

## Tier 1: Hardware Adapter Transports

**Definition:** Physical layer adapters that extend or transform Tier 0 transports. Require external circuitry or signaling conversion.

**Examples:**
- **RS485** — Half-duplex differential signaling (over UART)
- **RS232** — Full-duplex point-to-point serial (over UART)
- **CAN** — Controller Area Network bus (requires external transceiver)
- **Ethernet** — Network interface (MAC layer, typically SPI-based)
- **RC_TRANSCEIVER** — Radio control signal encoding/decoding

**Characteristics:**
- Built on Tier 0 transports (e.g., RS485 uses UART + direction control pin)
- Physical layer only (no protocol semantics)
- May require external hardware (transceivers, level shifters)
- Provide electrical/signal conditioning

**Example Endpoint Names:**
- `rs485_0` (RS485 bus 0, built on uart1 + gpio.pin.12)
- `can0` (CAN bus 0, requires external CAN transceiver)
- `eth0` (Ethernet interface 0)

---

## Tier 2: Radio/MAC Surface Transports

**Definition:** Wireless MAC (Media Access Control) layer interfaces. Provide network-like abstractions.

**Examples:**
- **WiFi** — IEEE 802.11 wireless networking
- **BLE** — Bluetooth Low Energy radio

**Characteristics:**
- Complex state machines (scanning, connecting, authenticating)
- Built-in protocol support (IP stack for WiFi, GATT for BLE)
- Platform-specific implementations
- Not all platforms support all Tier 2 transports (e.g., RP2040 has no built-in WiFi)

**Example Endpoint Names:**
- `wifi0` (WiFi interface)
- `ble0` (BLE radio)

---

## Non-Transports: What Does NOT Belong Here

### Device Drivers (Use DRIVER_CATALOG.md)
Device drivers are **chip/module implementations** that bind to transport endpoints:
- **MCP2515** — SPI-based CAN controller chip (driver binds to `spi0:cs=5`)
- **nRF24L01** — SPI-based 2.4GHz transceiver (driver binds to `spi0:cs=10`)
- **SX127x/RFM95W** — SPI-based LoRa radio module (driver binds to `spi0:cs=8`)
- **BME280** — I2C/SPI environmental sensor (driver binds to `i2c0:addr=0x76`)

**Key difference:** Drivers **consume** transports; transports **provide** plumbing.

When a driver initializes, it may **publish a virtual transport endpoint**:
- MCP2515 driver publishes `can0` (virtual CAN transport)
- nRF24L01 driver publishes `nrf24_0` (virtual 2.4GHz link)
- SX127x driver publishes `lora_phy0` (virtual LoRa physical layer)

See **docs/DRIVER_CATALOG.md** for driver taxonomy.

### Protocol Layers (Use PROTOCOL_LAYERS.md)
Protocol layers provide **semantic structure** over transports/virtual transports:
- **LoRaWAN** — Network protocol over LoRa PHY (over `lora_phy0` virtual transport)
- **Modbus RTU** — Industrial protocol over RS485 (over `rs485_0` transport)
- **CANopen** — Application layer over CAN (over `can0` transport)
- **MQTT** — Message broker protocol over WiFi/Ethernet (over `wifi0` or `eth0`)

See **docs/PROTOCOL_LAYERS.md** for protocol taxonomy.

---

## Transport Lifecycle

All transports implement the `TransportBase` interface:

```cpp
class TransportBase {
public:
    virtual bool init() = 0;        // Initialize transport
    virtual bool deinit() = 0;      // Shutdown transport
    virtual bool reset();           // Reset (deinit + init)
    
    State getState() const;         // UNINITIALIZED, INITIALIZING, READY, ERROR, DISABLED
    bool isReady() const;           // Check if transport is operational
    
    Status getStatus() const;       // Get detailed status (uptime, error counts, etc.)
};
```

---

## Usage Examples

### 1. Query available transports
```
> transport list
gpio0      TIER0  GPIO    READY
adc0       TIER0  ADC     READY
i2c0       TIER0  I2C     READY
spi0       TIER0  SPI     READY
uart1      TIER0  UART    READY
wifi0      TIER2  WIFI    READY
```

### 2. Check transport status
```
> transport info i2c0
Name:   i2c0
Type:   I2C
Tier:   TIER0
State:  READY
Uptime: 3600000 ms
Errors: 0
```

### 3. Configure transport
```
> transport config i2c0 frequency 400000
I2C frequency set to 400kHz
```

---

## Summary

| Tier | Purpose | Examples | Built On |
|------|---------|----------|----------|
| **TIER0** | MCU peripherals | GPIO, ADC, PWM, I2C, SPI, UART, OneWire | Hardware |
| **TIER1** | Physical adapters | RS485, RS232, CAN, Ethernet, RC | TIER0 + external circuitry |
| **TIER2** | Wireless MAC | WiFi, BLE | Hardware + firmware |

**Remember:**
- Transports = **plumbing** (byte/signal pipes + resource ownership)
- Drivers = **device chips** (MCP2515, nRF24L01, SX127x)
- Protocols = **semantics** (LoRaWAN, Modbus RTU, CANopen)

---

**Last Updated:** 2026-02-09  
**See Also:**
- `docs/DRIVER_CATALOG.md` — Device driver taxonomy
- `docs/PROTOCOL_LAYERS.md` — Protocol layer taxonomy
- `src/pocketos/transport/transport_base.h` — TransportBase interface
