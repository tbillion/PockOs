# Driver Catalog — PocketOS Device Driver Taxonomy

## Purpose
This document defines the **device driver taxonomy** in PocketOS. Drivers are chip/module implementations that bind to transport endpoints and optionally publish virtual transport endpoints.

---

## Core Definition: What is a Driver?

A **device driver** is:
- A **software abstraction** for a specific hardware chip or module
- **Binds to** one or more transport endpoints (e.g., `spi0:cs=5`, `i2c0:addr=0x76`)
- May **publish virtual transport endpoints** (e.g., MCP2515 publishes `can0`)
- Implements the `IDriver` interface (see `src/pocketos/core/driver_interface.h`)

**Key Characteristics:**
- Drivers **consume** transports (they require plumbing to communicate)
- Drivers **may produce** virtual transports (they extend the transport surface)
- Drivers are **device-specific** (one driver per chip/module family)

---

## Driver Lifecycle

All drivers implement the `IDriver` interface:

```cpp
class IDriver {
public:
    virtual bool bind(const std::string& endpointSpec) = 0;  // Bind to transport endpoint
    virtual bool unbind() = 0;                                // Release transport resources
    virtual bool configure(const std::map<std::string, std::string>& params) = 0;
    virtual std::string getState() const = 0;                 // "unbound", "bound", "active", "error"
    virtual std::string getHealth() const = 0;                // Health status
    virtual std::string getCapabilities() const = 0;          // Capability schema
};
```

---

## Virtual Transport Concept

Some drivers **publish virtual transport endpoints** after successful initialization:

1. **Driver binds** to a physical transport (e.g., MCP2515 binds to `spi0:cs=5`)
2. **Driver initializes** the hardware chip
3. **Driver publishes** a virtual transport endpoint (e.g., `can0`)
4. **Other code** can now use `can0` as if it were a physical transport

**Example Flow:**
```
1. User binds MCP2515 driver to spi0:cs=5
   > dev bind mcp2515_0 drv_mcp2515 spi0:cs=5

2. MCP2515 driver initializes the chip

3. MCP2515 driver publishes "can0" virtual transport

4. CANopen protocol layer can now bind to can0
   > protocol bind canopen_0 proto_canopen can0
```

**Benefits:**
- Clean abstraction layers (protocol code doesn't know if `can0` is physical or virtual)
- Flexibility (swap MCP2515 for native ESP32 TWAI controller without changing protocol code)
- Composability (drivers can stack: BLE UART publishes `uart_ble0`, which NMEA parser binds to)

---

## Driver Categories

### 1. Communication Interface Drivers

#### MCP2515 (CAN Controller)
**Purpose:** SPI-based CAN 2.0B controller chip  
**Binding:** `spi0:cs=5` (requires SPI bus + chip select pin)  
**Virtual Transport Published:** `can0` (CAN bus)  
**Status:** PLANNED (not yet implemented)

**Configuration Parameters:**
- `bitrate` — CAN bitrate (125k, 250k, 500k, 1M)
- `mode` — Operating mode (normal, loopback, listen-only)
- `clock` — External clock frequency (8MHz, 16MHz)

**Example:**
```
> dev bind mcp2515_0 drv_mcp2515 spi0:cs=5
> dev param mcp2515_0 bitrate 500000
> dev param mcp2515_0 mode normal
```

---

#### nRF24L01+ (2.4GHz Transceiver)
**Purpose:** SPI-based 2.4GHz wireless transceiver  
**Binding:** `spi0:cs=10` (requires SPI bus + CE/CSN pins)  
**Virtual Transport Published:** `nrf24_0` (2.4GHz radio link)  
**Status:** PLANNED (not yet implemented)

**Configuration Parameters:**
- `channel` — RF channel (0-125)
- `data_rate` — 250kbps, 1Mbps, 2Mbps
- `pa_level` — Power amplifier level (MIN, LOW, HIGH, MAX)
- `payload_size` — Dynamic or fixed (1-32 bytes)
- `pipe_address` — RX/TX pipe address (5 bytes)

**Example:**
```
> dev bind nrf24_0 drv_nrf24l01 spi0:cs=10
> dev param nrf24_0 channel 76
> dev param nrf24_0 data_rate 2Mbps
> dev param nrf24_0 pa_level MAX
```

---

#### SX127x/RFM95W (LoRa Radio)
**Purpose:** SPI-based LoRa/FSK radio module  
**Binding:** `spi0:cs=8` (requires SPI bus + reset/DIO pins)  
**Virtual Transport Published:** `lora_phy0` (LoRa physical layer)  
**Status:** PLANNED (not yet implemented)

**Configuration Parameters:**
- `frequency` — RF frequency (433MHz, 868MHz, 915MHz)
- `spreading_factor` — SF7-SF12
- `bandwidth` — 125kHz, 250kHz, 500kHz
- `coding_rate` — 4/5, 4/6, 4/7, 4/8
- `tx_power` — 2-20 dBm
- `preamble_length` — 6-65535 symbols

**Example:**
```
> dev bind lora_0 drv_sx127x spi0:cs=8
> dev param lora_0 frequency 915000000
> dev param lora_0 spreading_factor 7
> dev param lora_0 bandwidth 125000
> dev param lora_0 tx_power 20
```

**Note:** SX127x publishes `lora_phy0`, which LoRaWAN stack binds to.

---

### 2. Sensor Drivers

#### BME280 (Environmental Sensor)
**Purpose:** I2C/SPI environmental sensor (temp, humidity, pressure)  
**Binding:** `i2c0:addr=0x76` or `spi0:cs=15`  
**Virtual Transport Published:** None (sensors don't publish transports)  
**Status:** IMPLEMENTED (see `src/pocketos/drivers/bme280_driver.cpp`)

**Configuration Parameters:**
- `mode` — Sleep, forced, normal
- `oversampling_temp` — x1, x2, x4, x8, x16
- `oversampling_humidity` — x1, x2, x4, x8, x16
- `oversampling_pressure` — x1, x2, x4, x8, x16
- `filter` — IIR filter coefficient (off, 2, 4, 8, 16)

**Example:**
```
> dev bind bme280_0 drv_bme280 i2c0:addr=0x76
> dev param bme280_0 mode normal
> dev read bme280_0 temperature
23.5°C
```

---

### 3. Display Drivers (Future)

#### SSD1306 (OLED Display)
**Purpose:** I2C/SPI OLED display controller  
**Binding:** `i2c0:addr=0x3C` or `spi0:cs=12`  
**Virtual Transport Published:** None  
**Status:** FUTURE

---

#### ILI9341 (TFT LCD)
**Purpose:** SPI TFT LCD display controller  
**Binding:** `spi0:cs=14`  
**Virtual Transport Published:** None  
**Status:** FUTURE

---

### 4. Motor/Actuator Drivers (Future)

#### DRV8825 (Stepper Motor)
**Purpose:** Stepper motor driver (STEP/DIR interface)  
**Binding:** `gpio.pin.16,gpio.pin.17` (step/direction pins)  
**Virtual Transport Published:** None  
**Status:** FUTURE

---

#### PCA9685 (PWM Controller)
**Purpose:** I2C 16-channel PWM/servo controller  
**Binding:** `i2c0:addr=0x40`  
**Virtual Transport Published:** `pwm_ext0` (virtual PWM transport with 16 channels)  
**Status:** FUTURE

---

## Driver Registration

Drivers are registered in the **Device Registry** at compile time or runtime:

```cpp
// Register driver factory
DeviceRegistry::registerDriver("drv_mcp2515", []() -> IDriver* {
    return new MCP2515Driver();
});
```

---

## Driver Binding Syntax

**Format:** `<transport>:<parameter>=<value>[,<parameter>=<value>...]`

**Examples:**
- `spi0:cs=5` — SPI bus 0, chip select pin 5
- `i2c0:addr=0x76` — I2C bus 0, device address 0x76
- `uart1:baudrate=9600` — UART port 1, 9600 baud
- `gpio.pin.16` — GPIO pin 16
- `can0` — Virtual CAN transport (published by MCP2515 driver)

---

## Driver State Machine

```
UNBOUND → (bind) → BOUND → (init) → ACTIVE
                              ↓ (error)
                            ERROR
                              ↓ (reset)
                            BOUND
```

**States:**
- `unbound` — Driver not bound to any transport endpoint
- `bound` — Driver bound to transport, not yet initialized
- `active` — Driver initialized and operational
- `error` — Driver encountered error during init or operation

---

## Virtual Transport Lifetime

Virtual transports are **tied to driver lifetime**:

1. Driver binds → virtual transport does NOT exist yet
2. Driver initializes → virtual transport is published
3. Driver unbinds → virtual transport is removed

**Example:**
```
> transport list
i2c0       TIER0  I2C     READY
spi0       TIER0  SPI     READY

> dev bind mcp2515_0 drv_mcp2515 spi0:cs=5
Binding mcp2515_0 to spi0:cs=5...
MCP2515 initialized successfully
Virtual transport "can0" published

> transport list
i2c0       TIER0  I2C     READY
spi0       TIER0  SPI     READY
can0       TIER1  CAN     READY (virtual, provided by mcp2515_0)

> dev unbind mcp2515_0
Unbinding mcp2515_0...
Virtual transport "can0" removed

> transport list
i2c0       TIER0  I2C     READY
spi0       TIER0  SPI     READY
```

---

## Driver Discovery and Probing

PocketOS supports **automatic driver discovery** via I2C bus scanning:

```
> ep probe i2c0
Probing I2C bus 0...
Found device at 0x76 (possible: BME280, BMP280)
Found device at 0x3C (possible: SSD1306 OLED)
```

---

## Planned Drivers (Roadmap)

### Communication
- [ ] MCP2515 (CAN controller)
- [ ] nRF24L01+ (2.4GHz transceiver)
- [ ] SX127x/RFM95W (LoRa radio)
- [ ] ESP-NOW (ESP32 peer-to-peer)
- [ ] HC-05/06 (Bluetooth classic UART)

### Sensors
- [x] BME280 (temp/humidity/pressure)
- [ ] MPU6050 (IMU: accelerometer + gyroscope)
- [ ] BMP280 (temp/pressure)
- [ ] DHT22 (temp/humidity)
- [ ] DS18B20 (1-Wire temperature)
- [ ] GPS (NMEA over UART)

### Displays
- [ ] SSD1306 (OLED 128x64)
- [ ] ILI9341 (TFT 240x320)
- [ ] ST7789 (TFT 240x240)

### Actuators
- [ ] DRV8825 (stepper motor)
- [ ] L298N (DC motor H-bridge)
- [ ] PCA9685 (16-channel PWM/servo)
- [ ] Servo (PWM control)

### Storage
- [ ] SD card (SPI mode)
- [ ] AT24C EEPROM (I2C)
- [ ] W25Q flash (SPI)

---

## Summary

| Aspect | Description |
|--------|-------------|
| **Purpose** | Device-specific chip/module abstraction |
| **Binding** | Consumes transport endpoints (e.g., `spi0:cs=5`) |
| **Virtual Transports** | May publish new transport endpoints (e.g., `can0`) |
| **Interface** | `IDriver` (bind, unbind, configure, getState, getHealth) |
| **Examples** | MCP2515 (CAN), nRF24L01 (2.4GHz), SX127x (LoRa), BME280 (sensor) |

**Remember:**
- Drivers are **NOT** transports (they use transports)
- Drivers **MAY** publish virtual transports (extending the transport surface)
- Drivers are **device-specific** (one driver per chip family)

---

**Last Updated:** 2026-02-09  
**See Also:**
- `docs/TRANSPORT_TIERS.md` — Transport layer taxonomy
- `docs/PROTOCOL_LAYERS.md` — Protocol layer taxonomy
- `src/pocketos/core/driver_interface.h` — IDriver interface
- `src/pocketos/drivers/` — Driver implementations
