# Tiered Transport Surfaces Implementation Plan

**Version:** 1.0  
**Date:** 2026-02-08  
**Status:** Planning Complete - Ready for Implementation

## Executive Summary

This document outlines the comprehensive implementation plan for PocketOS Tiered Transport Surfaces. The goal is to create a unified, hierarchical transport layer that provides hardware-level interfaces for all supported communication methods across ESP32, ESP8266, and RP2040 platforms.

## Transport Tier Architecture

### Tier 0: Basic Hardware Transports
**Definition:** Direct hardware interfaces with minimal abstraction.

**Transports:**
- **GPIO:** Digital input/output (din/dout)
- **ADC:** Analog-to-digital conversion
- **PWM:** Pulse-width modulation output
- **I2C Master:** Inter-IC communication bus (already implemented)
- **SPI Master:** Serial Peripheral Interface bus (already implemented)
- **UART:** Universal Asynchronous Receiver-Transmitter (already implemented)
- **OneWire:** 1-Wire bus protocol (already implemented)

**Implementation Status:**
- I2C, SPI, UART, OneWire: ✅ Complete (38,492 bytes)
- GPIO, ADC, PWM: ❌ Need implementation (~1,200 lines)

### Tier 1: Hardware Adapters
**Definition:** Hardware-layer protocols built on Tier0, no high-level services.

**Transports:**
- **RS485:** UART + DE/RE pin control + turnaround timing
- **RS232:** UART + level shift config wrapper
- **CAN:** Hardware CAN controller access
  - ESP32: TWAI (Two-Wire Automotive Interface) controller
  - ESP8266/RP2040: Unsupported (no native controller)
- **Ethernet:** PHY/MAC hardware layer (no IP stack)
  - ESP32 variants: RMII/MDIO/MDC + PHY reset + link status
  - ESP8266/RP2040: Unsupported

**Implementation Status:**
- All Tier1: ❌ Need implementation (~1,800 lines)

### Tier 2: Radio/MAC Surfaces
**Definition:** Wireless hardware control (enable/disable + status), no connectivity services.

**Transports:**
- **WiFi:** Hardware control + status
  - ESP32/ESP8266: Supported (enable/disable, power, MAC, state)
  - RP2040: Pico W variant only
- **BLE/Bluetooth:** Hardware control + status
  - ESP32: Supported (enable/disable, power, address, state)
  - ESP8266/RP2040: Unsupported

**Implementation Status:**
- All Tier2: ❌ Need implementation (~1,200 lines)

## Platform Support Matrix

| Transport | ESP32 | ESP8266 | RP2040 | Notes |
|-----------|-------|---------|--------|-------|
| **Tier 0** |
| GPIO      | ✅ | ✅ | ✅ | Universal |
| ADC       | ✅ | ✅ (1ch) | ✅ | ESP8266 limited |
| PWM       | ✅ | ✅ | ✅ | Channel counts vary |
| I2C       | ✅ | ✅ (master) | ✅ | ESP8266 no slave |
| SPI       | ✅ | ✅ | ✅ | Universal |
| UART      | ✅ (3) | ✅ (2) | ✅ (2) | Port counts vary |
| OneWire   | ✅ | ✅ | ✅ | Bit-bang |
| **Tier 1** |
| RS485     | ✅ | ✅ | ✅ | UART + GPIO |
| RS232     | ✅ | ✅ | ✅ | UART wrapper |
| CAN       | ✅ (TWAI) | ❌ | ❌ | ESP32 only |
| Ethernet  | ✅ (variants) | ❌ | ❌ | ESP32 only |
| **Tier 2** |
| WiFi      | ✅ | ✅ | ✅ (Pico W) | ESP required |
| BLE       | ✅ | ❌ | ❌ | ESP32 only |

## Implementation Phases

### Phase 1: Transport Base Architecture (~500 lines)

**Files to Create:**
- `src/pocketos/transport/transport_base.h` (~150 lines)
- `src/pocketos/transport/transport_base.cpp` (~100 lines)
- `src/pocketos/transport/transport_registry.h` (~100 lines)
- `src/pocketos/transport/transport_registry.cpp` (~150 lines)

**Key Classes:**
```cpp
class TransportBase {
public:
    enum class Tier { TIER0, TIER1, TIER2 };
    enum class State { UNINITIALIZED, INITIALIZING, READY, ERROR, DISABLED };
    
    virtual bool init() = 0;
    virtual bool deinit() = 0;
    virtual State getState() const = 0;
    virtual bool isSupported() const = 0;
    virtual const char* getName() const = 0;
    virtual Tier getTier() const = 0;
    
    // Configuration
    virtual bool configure(const char* key, const char* value) = 0;
    virtual bool getConfig(const char* key, char* value, size_t maxLen) const = 0;
    
    // Status
    virtual bool getStatus(char* buffer, size_t maxLen) const = 0;
    virtual bool reset() = 0;
};

class TransportRegistry {
public:
    static TransportRegistry& getInstance();
    
    bool registerTransport(TransportBase* transport);
    TransportBase* getTransport(const char* name);
    int getTransportCount() const;
    const char** getTransportNames() const;
    
    // Tier filtering
    int getTransportsByTier(Tier tier, TransportBase** out, int maxCount);
};
```

### Phase 2: Tier0 Completion (~1,200 lines)

**GPIO Transport:**
- `src/pocketos/transport/gpio_transport.h` (~200 lines)
- `src/pocketos/transport/gpio_transport.cpp` (~400 lines)
- Per-platform implementations (ESP32/ESP8266/RP2040)
- Digital input/output with configurable pins
- Pull-up/pull-down/floating modes
- Interrupt support where available

**ADC Transport:**
- `src/pocketos/transport/adc_transport.h` (~150 lines)
- `src/pocketos/transport/adc_transport.cpp` (~300 lines)
- Per-platform ADC access
- Channel configuration
- Resolution settings
- Attenuation (ESP32)

**PWM Transport:**
- `src/pocketos/transport/pwm_transport.h` (~150 lines)
- `src/pocketos/transport/pwm_transport.cpp` (~350 lines)
- Per-platform PWM generation
- Frequency and duty cycle control
- Multiple channels

### Phase 3: Tier1 Implementation (~1,800 lines)

**RS485 Transport:**
- `src/pocketos/transport/rs485_transport.h` (~200 lines)
- `src/pocketos/transport/rs485_transport.cpp` (~400 lines)
- UART-based with DE/RE control
- Turnaround timing
- Collision detection

**RS232 Transport:**
- `src/pocketos/transport/rs232_transport.h` (~100 lines)
- `src/pocketos/transport/rs232_transport.cpp` (~200 lines)
- UART wrapper with config
- Hardware flow control support

**CAN Transport:**
- `src/pocketos/transport/can_transport.h` (~250 lines)
- `src/pocketos/transport/can_transport.cpp` (~500 lines)
- ESP32 TWAI controller
- Frame TX/RX (no protocol)
- Error counters
- Unsupported on ESP8266/RP2040

**Ethernet Transport:**
- `src/pocketos/transport/eth_transport.h` (~150 lines)
- `src/pocketos/transport/eth_transport.cpp` (~350 lines)
- ESP32 PHY/MAC layer
- Link status detection
- PHY reset control
- Unsupported on ESP8266/RP2040

### Phase 4: Tier2 Implementation (~1,200 lines)

**WiFi Transport:**
- `src/pocketos/transport/wifi_transport.h` (~200 lines)
- `src/pocketos/transport/wifi_transport.cpp` (~600 lines)
- Enable/disable radio
- TX power control
- MAC address access
- Connection state (no services)
- Per-platform (ESP32, ESP8266, Pico W)

**BLE Transport:**
- `src/pocketos/transport/ble_transport.h` (~150 lines)
- `src/pocketos/transport/ble_transport.cpp` (~250 lines)
- Enable/disable radio
- TX power control
- BT address access
- State reporting (no GATT)
- ESP32 only

### Phase 5: PCF1 Extension (~800 lines)

**PCF1 Transport Configuration:**
```ini
[transport:i2c0]
type=i2c
sda=21
scl=22
speed_hz=400000
enabled=true

[transport:uart1]
type=uart
tx=17
rx=16
baud=115200
enabled=true

[transport:rs4850]
type=rs485
uart=uart1
de_pin=4
re_pin=4
turnaround_us=500
enabled=true

[transport:wifi0]
type=wifi
enabled=true
tx_power_dbm=20
```

**Files to Modify:**
- `src/pocketos/core/pcf1_config.h` (+100 lines)
- `src/pocketos/core/pcf1_config.cpp` (+700 lines)

**Features:**
- Transport section parsing
- Validation (pins safe, no conflicts)
- Type-specific validation
- Per-platform capability checking

### Phase 6: Intent API & CLI (~600 lines)

**New Intent Handlers:**
- `transport.list` - List all transports
- `transport.info <name>` - Get transport details
- `transport.config <name> key=value` - Configure transport
- `transport.reset <name>` - Reset transport
- `transport.status <name>` - Get status/counters

**Files to Modify:**
- `src/pocketos/core/intent_api.h` (+50 lines)
- `src/pocketos/core/intent_api.cpp` (+300 lines)
- `src/pocketos/cli/cli.cpp` (+250 lines)

### Phase 7: Capability Reporting (~200 lines)

**Files to Modify:**
- `src/pocketos/platform/platform_pack.h` (+50 lines)
- `src/pocketos/platform/esp32_platform.cpp` (+50 lines)
- `src/pocketos/platform/esp8266_platform.cpp` (+50 lines)
- `src/pocketos/platform/rp2040_platform.cpp` (+50 lines)

**Features:**
- Report transport availability per platform
- CAN availability (ESP32 only)
- Ethernet availability (ESP32 variants)
- BLE availability (ESP32 only)

### Phase 8: CI & Quality (~100 lines)

**Files to Modify:**
- `.github/workflows/build.yml` (+50 lines)

**Features:**
- Stub detection in transport/
- Build matrix verification
- Quality gates

### Phase 9: Documentation (~4,000 lines)

**Files to Create:**
- `docs/TRANSPORT_TIERS.md` (~1,500 lines)
  - Tier definitions
  - Platform support matrix
  - Usage guidelines
  
- `docs/TRANSPORT_PCF1.md` (~1,500 lines)
  - PCF1 configuration blocks
  - Validation rules
  - Examples per transport type
  
- `docs/TRANSPORT_DEMO.md` (~1,000 lines)
  - CLI transcripts
  - Configuration examples
  - Status reporting examples

## Implementation Timeline

**Session 1 (Current):**
- ✅ Planning and architecture
- ✅ Session tracking
- Phase 1: Transport base (if time permits)

**Session 2:**
- Phase 1: Complete base architecture
- Phase 2: Tier0 completion (GPIO, ADC, PWM)
- Begin Phase 3: Tier1 (RS485, RS232)

**Session 3:**
- Complete Phase 3: Tier1 (CAN, Ethernet)
- Phase 4: Tier2 (WiFi, BLE)
- Phase 5: PCF1 extension

**Session 4:**
- Phase 6: Intent API & CLI
- Phase 7: Capability reporting
- Phase 8: CI updates
- Phase 9: Documentation

## Definition of Done Checklist

- [ ] All 3 platform builds pass (esp32dev, d1_mini, pico)
- [ ] Tier0 transports fully implemented
- [ ] Tier1 transports implemented where hardware exists
- [ ] Tier2 transports implemented with hardware control only
- [ ] Capability reporting is accurate and target-aware
- [ ] PCF1 extended for transport configuration
- [ ] CLI commands for transport management (via IntentAPI)
- [ ] No stubs in transport/platform directories
- [ ] CI stub detection enforced
- [ ] TRANSPORT_TIERS.md created
- [ ] TRANSPORT_PCF1.md created
- [ ] TRANSPORT_DEMO.md created
- [ ] Session tracking complete
- [ ] Roadmap updated (append-only)

## Risk Assessment

**High Risk:**
- Large scope may require multiple sessions
- Platform-specific WiFi/BLE APIs vary significantly
- CAN/Ethernet only on specific platforms

**Medium Risk:**
- PCF1 validation complexity
- Transport initialization ordering
- Resource conflict detection

**Low Risk:**
- Tier0 completion (well-understood)
- CI integration (existing pattern)
- Documentation (clear structure)

## Success Metrics

- **Code Quality:** Zero stubs, complete implementations
- **Platform Coverage:** All 3 platforms build successfully
- **Capability Accuracy:** Platforms report only what they support
- **Configuration:** PCF1 round-trip works for all transports
- **Documentation:** Complete usage examples and transcripts

---

**Status:** READY FOR IMPLEMENTATION  
**Estimated Total:** ~10,100 lines across 30+ files  
**Priority:** HIGH - Core infrastructure for hardware abstraction
