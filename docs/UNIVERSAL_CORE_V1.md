# PocketOS Universal Core v1

## Overview

PocketOS Universal Core v1 represents a complete embedded device management platform with:
- **OS-Center Device Manager** - Complete device lifecycle management
- **PCF1 Configuration** - Human-readable config with validation
- **Platform Packs** - Multi-platform support (ESP32/ESP8266/RP2040)
- **Service Model** - Deterministic scheduler with core services
- **Enhanced Logging** - Structured logging with telemetry
- **Safety Defaults** - Safe outputs and config validation

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      PocketOS CLI                            │
│                 (Serial Console Interface)                   │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                  Intent API v1.0.0                           │
│            (28 Intent Handlers + Error Model)                │
└──────────────────────┬──────────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┬──────────────┐
        │              │               │              │
┌───────▼──────┐ ┌────▼─────┐ ┌──────▼──────┐ ┌────▼─────┐
│  Platform    │ │ Service  │ │    PCF1     │ │  Device  │
│    Pack      │ │ Manager  │ │   Config    │ │ Registry │
│              │ │          │ │             │ │          │
│ ESP32 ✓      │ │ Health   │ │ Validate    │ │ Bind     │
│ ESP8266 ✓    │ │ Telemetry│ │ Import      │ │ Unbind   │
│ RP2040 ✓     │ │ Persist  │ │ Export      │ │ Status   │
└──────────────┘ └──────────┘ └─────────────┘ └──────────┘
        │              │               │              │
        └──────────────┼───────────────┼──────────────┘
                       │               │
              ┌────────▼───────┬───────▼────────┐
              │     HAL        │  Persistence   │
              │  (Hardware)    │    (NVS)       │
              └────────────────┴────────────────┘
```

## Core Components

### 1. Intent API (Control Vocabulary)

**28 Intent Handlers:**
- System: sys.info
- HAL: hal.caps
- Endpoints: ep.list, ep.probe
- Devices: dev.list, dev.bind, dev.unbind, dev.enable, dev.disable, dev.status, dev.read, dev.stream
- Parameters: param.get, param.set
- Schema: schema.get
- Logging: log.tail, log.clear
- Persistence: persist.save, persist.load
- Config: config.export, config.import, config.validate
- Bus: bus.list, bus.info, bus.config
- Identification: identify
- Factory: factory_reset

**Error Model (7 stable codes):**
- OK
- ERR_BAD_ARGS
- ERR_NOT_FOUND
- ERR_CONFLICT
- ERR_IO
- ERR_UNSUPPORTED
- ERR_INTERNAL

### 2. PCF1 Configuration Format

Human-readable text configuration with sections:

```
[system]
version=1.0.0
platform=esp32

[i2c0]
sda=21
scl=22
speed_hz=400000

[device:1]
endpoint=i2c0:0x76
driver=bme280
state=enabled
```

**Features:**
- Line-oriented key=value format
- Section-based organization
- Validation with detailed errors
- Import/export with round-trip
- Factory reset support

### 3. Platform Packs

**Platform Contract:**
- Platform identification
- Hardware capabilities
- Memory management
- GPIO management
- Persistence options
- Power management
- Reset handling

**Supported Platforms:**
1. **ESP32** - Full implementation
   - WiFi + Bluetooth
   - I2C, SPI, ADC, PWM
   - NVS persistence
   - Deep sleep support

2. **ESP8266** - Stub (compiles)
   - WiFi only
   - I2C, SPI, ADC, PWM
   - EEPROM persistence
   - Basic sleep

3. **RP2040** - Stub (compiles)
   - No WiFi (Pico W variant has WiFi)
   - I2C, SPI, ADC, PWM
   - Flash persistence
   - Basic power management

### 4. Service Model

**Deterministic Tick-Based Scheduler:**
- Services run on fixed tick intervals
- Predictable execution timing
- No time-drift accumulation

**Core Services:**
1. **Health Service** (every 1000 ticks)
   - Memory monitoring
   - Device health checks
   - System health reports

2. **Telemetry Service** (every 500 ticks)
   - Counter collection
   - Gauge recording
   - Telemetry reports

3. **Persistence Service** (every 6000 ticks)
   - Auto-save on request
   - Config persistence
   - State preservation

### 5. Device Registry

**Device Lifecycle:**
1. Scan/Probe endpoints
2. Identify devices (auto-ID where possible)
3. Bind driver to endpoint
4. Configure parameters
5. Enable/disable
6. Monitor status/health
7. Persist configuration
8. Auto-restore on boot

**Health Tracking:**
- Init fail count
- I/O fail count
- Last OK timestamp
- Device state (READY/FAULT/DISABLED)

### 6. Safety Defaults

**Safety Features:**
- Outputs default to safe state (LOW/OFF)
- Actuator drivers require explicit enable
- Config validation prevents unsafe states
- Resource conflict detection
- Safe pin identification per platform
- Factory reset with confirmation

## Boot Sequence

```
1. Platform Pack initialization
2. Core system initialization
   - Logger
   - HAL
   - Intent API
   - Resource Manager
   - Endpoint Registry
   - Device Registry
   - Device Identifier
   - Persistence
   - PCF1 Config
3. Service Manager initialization
4. Core services registration and start
   - Health
   - Telemetry
   - Persistence
5. Configuration loading
6. CLI initialization
7. Ready for user input
```

## CLI Command Reference

**System Commands:**
- `sys info` - System information
- `hal caps` - Hardware capabilities
- `help` - Command list

**Device Manager:**
- `ep list` - List endpoints
- `ep probe <endpoint>` - Probe/scan endpoint
- `identify <endpoint>` - Auto-identify device
- `bind <driver> <endpoint>` - Bind driver
- `unbind <device_id>` - Unbind device
- `enable <device_id>` - Enable device
- `disable <device_id>` - Disable device
- `dev list` - List devices
- `status <device_id>` - Device status
- `read <device_id>` - Read sensor data
- `stream <device_id> <interval> <count>` - Stream readings

**Configuration:**
- `config export` - Export config to PCF1
- `config import <data>` - Import PCF1 config
- `config validate <data>` - Validate config
- `factory_reset` - Clear all config

**Parameters:**
- `param set <dev_id> <name> <value>` - Set parameter
- `param get <dev_id> <name>` - Get parameter
- `schema <device_id>` - Show device schema

**Bus Management:**
- `bus list` - List available buses
- `bus info <bus>` - Bus information
- `bus config <bus> <params>` - Configure bus

**Services:**
- `service list` - List services
- `service start <name>` - Start service
- `service stop <name>` - Stop service
- `service status <name>` - Service status

**Logging:**
- `log tail [n]` - Show last n log entries
- `log clear` - Clear log buffer

**Persistence:**
- `persist save` - Save configuration
- `persist load` - Load configuration

## Code Statistics

**Total Implementation:**
- Source files: 37 (.cpp + .h)
- Total lines: ~5,500
- Core modules: 13
- Intent handlers: 28
- Services: 3
- Platforms: 3
- Drivers: 2 (GPIO DOUT, BME280)

**New in Universal Core v1:**
- PCF1 Config: 7,430 bytes
- Service Manager: 8,630 bytes
- Platform Packs: 8,711 bytes
- Total new: 24,771 bytes

## Build Configurations

**esp32dev (default):**
```ini
build_flags = 
    -DPOCKETOS_ENABLE_I2C
    -DPOCKETOS_ENABLE_ADC
    -DPOCKETOS_ENABLE_PWM
    -DPOCKETOS_DRIVER_PACKAGE=2  ; STANDARD tier
```

**esp32dev-minimal:**
```ini
build_flags = 
    -DPOCKETOS_ENABLE_I2C
    -DPOCKETOS_ENABLE_ADC
    -DPOCKETOS_ENABLE_PWM
    -DPOCKETOS_DRIVER_PACKAGE=1  ; MINIMAL tier
```

**esp32dev-full:**
```ini
build_flags = 
    -DPOCKETOS_ENABLE_I2C
    -DPOCKETOS_ENABLE_ADC
    -DPOCKETOS_ENABLE_PWM
    -DPOCKETOS_DRIVER_PACKAGE=3  ; FULL tier
```

## Memory Footprint

**Estimated sizes (ESP32):**
- Core system: ~50KB flash, ~8KB RAM
- Standard drivers: ~15KB per driver
- Services: ~5KB total
- Platform pack: ~3KB
- PCF1 config: ~2KB

**Total estimated:**
- Flash: ~75-90KB
- RAM: ~12-15KB
- NVS: Variable (user config)

## Extension Points

**Adding New Platforms:**
1. Create platform pack class
2. Implement PlatformPack interface
3. Add to createPlatformPack() factory
4. Test compilation

**Adding New Services:**
1. Extend Service base class
2. Implement init/tick/shutdown
3. Register with ServiceManager
4. Configure tick interval

**Adding New Drivers:**
1. Implement IDriver interface
2. Add to createDriver() factory
3. Implement capability schema
4. Add identification if applicable

**Adding New Intents:**
1. Add handler function
2. Add to dispatch() switch
3. Document in vocabulary
4. Test via CLI

## Future Enhancements

**Not in v1 (by design):**
- GUI/LVGL rendering
- Web interface
- HTTP server
- Macro/graph engine
- IPO (Intent Processing Objects)
- Multiple additional drivers

**Possible v2 features:**
- Remote configuration
- Over-the-air updates
- Advanced diagnostics
- Performance profiling
- Network services
- Additional bus types

## License & Credits

PocketOS Universal Core v1  
Copyright © 2026  
Open source embedded device management platform

---

**Version:** 1.0.0  
**Release Date:** 2026-02-08  
**Status:** Production Ready  
**Documentation:** Complete
