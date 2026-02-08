# Zero-Stub Universal Portability Implementation Plan

## Executive Summary

This document provides a comprehensive implementation plan for eliminating ALL stubs in PocketOS platform and transport layers across ESP32, ESP8266, and RP2040 targets.

**Estimated Scope:** 8,000-10,000 lines of code across 40+ new files
**Estimated Effort:** 40-60 hours of focused development
**Complexity:** High - requires deep knowledge of 3 different microcontroller architectures

## Mission

Eliminate ALL stubs in platform/transport layers by implementing complete per-target backends for ESP32, ESP8266, and RP2040. "Complete" means: real code paths per target, compile-gated; if a capability truly cannot exist on a target, it must be reported as not supported by Platform Pack (NOT a stub implementation).

## Definition of Done

1. ✅ PlatformIO builds pass for ALL targets:
   - `pio run -e esp32dev`
   - `pio run -e d1_mini` (ESP8266)
   - `pio run -e pico` (RP2040)

2. ✅ No "stub" code remains in src/pocketos/platform/ or src/pocketos/transport/:
   - Remove or replace any TODO/STUB placeholders
   - CI check fails if "STUB", "TODO: stub", "NOT_IMPLEMENTED" appears

3. ✅ Platform Pack is REAL on all targets:
   - Accurate safe pin tables / reserved pins
   - ADC + PWM mapping rules
   - Storage backend implemented
   - Reboot/watchdog hooks (as available)
   - Capability detection accurate

4. ✅ Transport surfaces are REAL on all targets:
   - I2CBusMaster implemented for ESP32/ESP8266/RP2040
   - SPIMaster implemented for ESP32/ESP8266/RP2040
   - UARTPort implemented for ESP32/ESP8266/RP2040
   - OneWireBus implemented for ESP32/ESP8266/RP2040
   - I2CBusSlave where feasible

5. ✅ Docs updated:
   - docs/PORTABILITY.md
   - docs/PCF1_SPEC.md updated
   
6. ✅ Config validate is target-aware

## Implementation Phases

### Phase 1: Project Structure (COMPLETED)

**Files Created:**
- `src/pocketos/platform/` directory
- `src/pocketos/transport/` directory

**Status:** ✅ Complete

### Phase 2: Platform Pack Implementation

#### 2.1 ESP32 Platform Pack Enhancement

**File:** `src/pocketos/platform/esp32_pack.cpp`

**Implementation Details:**
```cpp
// Safe pins for ESP32 DevKit
const uint8_t SAFE_PINS_ESP32[] = {
    4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
};

// Reserved/unsafe pins
const uint8_t RESERVED_PINS_ESP32[] = {
    0,  // Boot button
    1,  // TX0
    2,  // Boot mode  
    3,  // RX0
    6, 7, 8, 9, 10, 11,  // Flash
    34, 35, 36, 39  // Input only
};

// ADC channels
const uint8_t ADC_PINS_ESP32[] = {
    32, 33, 34, 35, 36, 39,  // ADC1
    4, 0, 2, 15, 13, 12, 14, 27, 25, 26  // ADC1 continued
};

// Capabilities
has_i2c_master = true;
has_i2c_slave = true;  // ESP32 supports I2C slave
has_spi_master = true;
has_uart = true;
uart_count = 3;
adc_channels = 18;
pwm_channels = 16;
has_onewire = true;

// Storage: Preferences/NVS (already implemented)
bool initStorage() {
    prefs.begin("pocketos", false);
    return true;
}
```

**Lines:** ~400
**Status:** ✅ Framework exists, needs enhancement

#### 2.2 ESP8266 Platform Pack

**File:** `src/pocketos/platform/esp8266_pack.cpp`

**Implementation Details:**
```cpp
#ifdef ESP8266
#include <LittleFS.h>
#include <ESP8266WiFi.h>

// Safe pins for ESP8266 (D1 Mini / NodeMCU)
const uint8_t SAFE_PINS_ESP8266[] = {
    D1, D2, D5, D6, D7  // GPIO5, GPIO4, GPIO14, GPIO12, GPIO13
};

// Reserved pins
const uint8_t RESERVED_PINS_ESP8266[] = {
    D0,  // GPIO16 - special, no interrupts
    D3,  // GPIO0 - boot mode
    D4,  // GPIO2 - boot mode / LED
    D8,  // GPIO15 - boot mode
    TX,  // GPIO1
    RX   // GPIO3
};

// ADC
adc_channels = 1;  // Only A0
const uint8_t ADC_PIN_ESP8266 = A0;

// Capabilities
has_i2c_master = true;
has_i2c_slave = false;  // ESP8266 does NOT support I2C slave
has_spi_master = true;
has_uart = true;
uart_count = 2;  // Hardware UART0 + UART1 TX-only
pwm_channels = 8;  // Software PWM on most pins
has_onewire = true;

// Storage: LittleFS
bool initStorage() {
    if (!LittleFS.begin()) {
        if (!LittleFS.format()) {
            return false;
        }
        if (!LittleFS.begin()) {
            return false;
        }
    }
    return true;
}

String readConfig(const char* key) {
    String path = String("/") + key + ".cfg";
    if (!LittleFS.exists(path)) {
        return "";
    }
    File f = LittleFS.open(path, "r");
    String value = f.readString();
    f.close();
    return value;
}

bool writeConfig(const char* key, const char* value) {
    String path = String("/") + key + ".cfg";
    File f = LittleFS.open(path, "w");
    if (!f) return false;
    f.print(value);
    f.close();
    return true;
}

void reboot() {
    ESP.restart();
}

void feedWatchdog() {
    ESP.wdtFeed();
}
#endif
```

**Lines:** ~600
**Status:** ❌ Needs implementation

#### 2.3 RP2040 Platform Pack

**File:** `src/pocketos/platform/rp2040_pack.cpp`

**Implementation Details:**
```cpp
#ifdef ARDUINO_ARCH_RP2040
#include <LittleFS.h>
#include <hardware/watchdog.h>

// Safe pins for RP2040 (Raspberry Pi Pico)
const uint8_t SAFE_PINS_RP2040[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22
};

// Reserved pins
const uint8_t RESERVED_PINS_RP2040[] = {
    23, 24, 25  // 25 is onboard LED, 23/24 are special
};

// ADC pins
const uint8_t ADC_PINS_RP2040[] = {
    26, 27, 28, 29  // ADC0-3
};

// Capabilities
has_i2c_master = true;
has_i2c_slave = true;  // RP2040 supports I2C slave
has_spi_master = true;
has_uart = true;
uart_count = 2;
adc_channels = 4;  // 3 external + 1 internal temp
pwm_channels = 16;  // All GPIO can PWM
has_onewire = true;

// Storage: LittleFS
bool initStorage() {
    if (!LittleFS.begin()) {
        // Try to format and retry
        LittleFS.format();
        if (!LittleFS.begin()) {
            return false;
        }
    }
    return true;
}

String readConfig(const char* key) {
    String path = String("/") + key + ".cfg";
    if (!LittleFS.exists(path)) {
        return "";
    }
    File f = LittleFS.open(path, "r");
    String value = f.readString();
    f.close();
    return value;
}

bool writeConfig(const char* key, const char* value) {
    String path = String("/") + key + ".cfg";
    File f = LittleFS.open(path, "w");
    if (!f) return false;
    f.print(value);
    f.close();
    return true;
}

void reboot() {
    watchdog_reboot(0, 0, 0);
}

void feedWatchdog() {
    // RP2040 watchdog feeding if enabled
}
#endif
```

**Lines:** ~600
**Status:** ❌ Needs implementation

### Phase 3: Transport Layer - I2C Master

#### 3.1 I2C Interface

**File:** `src/pocketos/transport/i2c_master.h`

```cpp
#ifndef POCKETOS_TRANSPORT_I2C_MASTER_H
#define POCKETOS_TRANSPORT_I2C_MASTER_H

#include <stdint.h>
#include <stddef.h>

namespace PocketOS {
namespace Transport {

class I2CMaster {
public:
    virtual ~I2CMaster() {}
    
    // Initialize I2C with SDA/SCL pins and speed
    virtual bool init(uint8_t sda_pin, uint8_t scl_pin, uint32_t speed_hz) = 0;
    
    // Scan for devices on the bus
    virtual bool scan(uint8_t* addresses, uint8_t max_count, uint8_t* found_count) = 0;
    
    // Read from device
    virtual bool read(uint8_t address, uint8_t* data, size_t length) = 0;
    
    // Write to device
    virtual bool write(uint8_t address, const uint8_t* data, size_t length) = 0;
    
    // Write then read (common pattern)
    virtual bool writeRead(uint8_t address, 
                          const uint8_t* write_data, size_t write_len,
                          uint8_t* read_data, size_t read_len) = 0;
    
    // Check if device responds
    virtual bool probe(uint8_t address) = 0;
    
    // Get current configuration
    virtual bool getConfig(uint8_t* sda, uint8_t* scl, uint32_t* speed) = 0;
};

// Factory function
I2CMaster* createI2CMaster();

} // namespace Transport
} // namespace PocketOS

#endif
```

**Lines:** ~50

#### 3.2 ESP32 I2C Implementation

**File:** `src/pocketos/transport/i2c_master_esp32.cpp`

```cpp
#ifdef ESP32
#include "i2c_master.h"
#include <Wire.h>

namespace PocketOS {
namespace Transport {

class I2CMasterESP32 : public I2CMaster {
private:
    TwoWire* wire;
    uint8_t sda_pin;
    uint8_t scl_pin;
    uint32_t speed_hz;
    bool initialized;

public:
    I2CMasterESP32() : wire(&Wire), initialized(false) {}
    
    bool init(uint8_t sda, uint8_t scl, uint32_t speed) override {
        sda_pin = sda;
        scl_pin = scl;
        speed_hz = speed;
        
        wire->begin(sda_pin, scl_pin, speed_hz);
        initialized = true;
        return true;
    }
    
    bool scan(uint8_t* addresses, uint8_t max_count, uint8_t* found_count) override {
        if (!initialized) return false;
        
        *found_count = 0;
        for (uint8_t addr = 1; addr < 127 && *found_count < max_count; addr++) {
            wire->beginTransmission(addr);
            if (wire->endTransmission() == 0) {
                addresses[(*found_count)++] = addr;
            }
        }
        return true;
    }
    
    bool read(uint8_t address, uint8_t* data, size_t length) override {
        if (!initialized) return false;
        
        size_t received = wire->requestFrom(address, length);
        if (received != length) return false;
        
        for (size_t i = 0; i < length; i++) {
            data[i] = wire->read();
        }
        return true;
    }
    
    bool write(uint8_t address, const uint8_t* data, size_t length) override {
        if (!initialized) return false;
        
        wire->beginTransmission(address);
        wire->write(data, length);
        return wire->endTransmission() == 0;
    }
    
    bool writeRead(uint8_t address,
                  const uint8_t* write_data, size_t write_len,
                  uint8_t* read_data, size_t read_len) override {
        if (!write(address, write_data, write_len)) return false;
        return read(address, read_data, read_len);
    }
    
    bool probe(uint8_t address) override {
        if (!initialized) return false;
        
        wire->beginTransmission(address);
        return wire->endTransmission() == 0;
    }
    
    bool getConfig(uint8_t* sda, uint8_t* scl, uint32_t* speed) override {
        if (!initialized) return false;
        *sda = sda_pin;
        *scl = scl_pin;
        *speed = speed_hz;
        return true;
    }
};

I2CMaster* createI2CMaster() {
    return new I2CMasterESP32();
}

} // namespace Transport
} // namespace PocketOS

#endif
```

**Lines:** ~100
**Status:** ❌ Needs implementation

#### 3.3 ESP8266 I2C Implementation

**File:** `src/pocketos/transport/i2c_master_esp8266.cpp`

Similar to ESP32 but with ESP8266-specific considerations:
- Wire library usage
- Pin limitations
- Speed limitations

**Lines:** ~100
**Status:** ❌ Needs implementation

#### 3.4 RP2040 I2C Implementation

**File:** `src/pocketos/transport/i2c_master_rp2040.cpp`

Similar structure using RP2040's Wire library.

**Lines:** ~100
**Status:** ❌ Needs implementation

### Phase 4: Transport Layer - SPI Master

#### 4.1 SPI Interface

**File:** `src/pocketos/transport/spi_master.h`

```cpp
#ifndef POCKETOS_TRANSPORT_SPI_MASTER_H
#define POCKETOS_TRANSPORT_SPI_MASTER_H

#include <stdint.h>
#include <stddef.h>

namespace PocketOS {
namespace Transport {

enum SPIMode {
    SPI_MODE0 = 0,
    SPI_MODE1 = 1,
    SPI_MODE2 = 2,
    SPI_MODE3 = 3
};

enum SPIBitOrder {
    SPI_MSBFIRST = 0,
    SPI_LSBFIRST = 1
};

class SPIMaster {
public:
    virtual ~SPIMaster() {}
    
    // Initialize SPI
    virtual bool init(uint8_t sck, uint8_t miso, uint8_t mosi, uint32_t speed_hz) = 0;
    
    // Transfer data (full duplex)
    virtual bool transfer(uint8_t cs_pin, const uint8_t* tx_data, uint8_t* rx_data, 
                         size_t length, SPIMode mode = SPI_MODE0, 
                         SPIBitOrder bit_order = SPI_MSBFIRST) = 0;
    
    // Write only
    virtual bool write(uint8_t cs_pin, const uint8_t* data, size_t length,
                      SPIMode mode = SPI_MODE0, SPIBitOrder bit_order = SPI_MSBFIRST) = 0;
    
    // Read only
    virtual bool read(uint8_t cs_pin, uint8_t* data, size_t length,
                     SPIMode mode = SPI_MODE0, SPIBitOrder bit_order = SPI_MSBFIRST) = 0;
};

// Factory function
SPIMaster* createSPIMaster();

} // namespace Transport
} // namespace PocketOS

#endif
```

**Lines:** ~60

#### 4.2-4.4 SPI Implementations

One implementation file for each platform (ESP32, ESP8266, RP2040), each ~150 lines.

**Total Lines:** ~450
**Status:** ❌ Needs implementation

### Phase 5: Transport Layer - UART

#### 5.1 UART Interface

**File:** `src/pocketos/transport/uart_port.h`

```cpp
#ifndef POCKETOS_TRANSPORT_UART_PORT_H
#define POCKETOS_TRANSPORT_UART_PORT_H

#include <stdint.h>
#include <stddef.h>

namespace PocketOS {
namespace Transport {

enum UARTParity {
    UART_PARITY_NONE = 0,
    UART_PARITY_EVEN = 1,
    UART_PARITY_ODD = 2
};

enum UARTStopBits {
    UART_STOP_BITS_1 = 0,
    UART_STOP_BITS_2 = 1
};

class UARTPort {
public:
    virtual ~UARTPort() {}
    
    // Initialize UART
    virtual bool init(uint8_t tx_pin, uint8_t rx_pin, uint32_t baud_rate,
                     UARTParity parity = UART_PARITY_NONE,
                     UARTStopBits stop_bits = UART_STOP_BITS_1) = 0;
    
    // Available bytes to read
    virtual size_t available() = 0;
    
    // Read data
    virtual size_t read(uint8_t* data, size_t max_length) = 0;
    
    // Write data
    virtual size_t write(const uint8_t* data, size_t length) = 0;
    
    // Flush
    virtual void flush() = 0;
};

// Factory function
UARTPort* createUARTPort(uint8_t port_number);

} // namespace Transport
} // namespace PocketOS

#endif
```

**Lines:** ~50

#### 5.2-5.4 UART Implementations

One implementation for each platform, ~120 lines each.

**Total Lines:** ~360
**Status:** ❌ Needs implementation

### Phase 6: Transport Layer - OneWire

#### 6.1 OneWire Interface

**File:** `src/pocketos/transport/onewire_bus.h`

```cpp
#ifndef POCKETOS_TRANSPORT_ONEWIRE_BUS_H
#define POCKETOS_TRANSPORT_ONEWIRE_BUS_H

#include <stdint.h>
#include <stddef.h>

namespace PocketOS {
namespace Transport {

class OneWireBus {
public:
    virtual ~OneWireBus() {}
    
    // Initialize OneWire on a pin
    virtual bool init(uint8_t pin) = 0;
    
    // Reset bus
    virtual bool reset() = 0;
    
    // Search for devices
    virtual bool search(uint64_t* rom_codes, uint8_t max_count, uint8_t* found_count) = 0;
    
    // Read ROM (for single device)
    virtual bool readROM(uint64_t* rom_code) = 0;
    
    // Select device
    virtual bool select(uint64_t rom_code) = 0;
    
    // Skip ROM (broadcast)
    virtual void skipROM() = 0;
    
    // Read byte
    virtual uint8_t read() = 0;
    
    // Write byte
    virtual void write(uint8_t data) = 0;
};

// Factory function
OneWireBus* createOneWireBus();

} // namespace Transport
} // namespace PocketOS

#endif
```

**Lines:** ~50

#### 6.2-6.4 OneWire Implementations

Can use OneWire library or bit-bang implementation. ~150 lines each.

**Total Lines:** ~450
**Status:** ❌ Needs implementation

### Phase 7: Integration

#### 7.1 Update HAL

**File:** `src/pocketos/core/hal.cpp`

- Replace direct Wire/SPI usage with Transport layer
- Update I2C functions to use I2CMaster
- Add transport factory methods

**Lines Modified:** ~200

#### 7.2 Update Device Drivers

**File:** `src/pocketos/drivers/bme280_driver.cpp`

- Use I2CMaster transport instead of Wire directly
- Make driver platform-agnostic

**Lines Modified:** ~50

#### 7.3 Update Intent API

Add transport-related intents if needed.

**Lines Modified:** ~100

#### 7.4 Update Main

Initialize transport layer.

**Lines Modified:** ~50

**Total Integration:** ~400 lines modified

### Phase 8: Build Configuration

#### 8.1 Update platformio.ini

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_flags = 
    -DPOCKETOS_DRIVER_PACKAGE=2
    -DPOCKETOS_ENABLE_I2C
    -DPOCKETOS_ENABLE_ADC
    -DPOCKETOS_ENABLE_PWM
lib_deps = 
    Wire
    SPI
    Preferences

[env:d1_mini]
platform = espressif8266
board = d1_mini
framework = arduino
monitor_speed = 115200
build_flags = 
    -DPOCKETOS_DRIVER_PACKAGE=2
    -DPOCKETOS_ENABLE_I2C
    -DPOCKETOS_ENABLE_ADC
    -DPOCKETOS_ENABLE_PWM
lib_deps = 
    Wire
    SPI
    LittleFS

[env:pico]
platform = raspberrypi
board = pico
framework = arduino
board_build.core = earlephilhower
monitor_speed = 115200
build_flags = 
    -DPOCKETOS_DRIVER_PACKAGE=2
    -DPOCKETOS_ENABLE_I2C
    -DPOCKETOS_ENABLE_ADC
    -DPOCKETOS_ENABLE_PWM
lib_deps = 
    Wire
    SPI
    LittleFS
```

**Lines:** ~80

#### 8.2 Test Builds

```bash
pio run -e esp32dev
pio run -e d1_mini
pio run -e pico
```

Fix compilation errors iteratively.

### Phase 9: CI/CD

#### 9.1 GitHub Actions Workflow

**File:** `.github/workflows/build.yml`

```yaml
name: PlatformIO CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        environment:
          - esp32dev
          - d1_mini
          - pico
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Cache pip
      uses: actions/cache@v3
      with:
        path: ~/.cache/pip
        key: ${{ runner.os }}-pip-${{ hashFiles('**/requirements.txt') }}
        restore-keys: |
          ${{ runner.os }}-pip-
    
    - name: Cache PlatformIO
      uses: actions/cache@v3
      with:
        path: ~/.platformio
        key: ${{ runner.os }}-${{ hashFiles('**/lockfiles') }}
    
    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
    
    - name: Install PlatformIO
      run: |
        python -m pip install --upgrade pip
        pip install --upgrade platformio
    
    - name: Build ${{ matrix.environment }}
      run: pio run -e ${{ matrix.environment }}
    
    - name: Check for stubs in platform/transport
      run: |
        if grep -r "STUB\|TODO: stub\|NOT_IMPLEMENTED" src/pocketos/platform/ src/pocketos/transport/ 2>/dev/null; then
          echo "ERROR: Found stub markers in platform or transport layers!"
          exit 1
        fi
        echo "No stub markers found - OK"
```

**Lines:** ~60

### Phase 10: Documentation

#### 10.1 PORTABILITY.md

**File:** `docs/PORTABILITY.md`

Content:
- Overview of 3 platform support
- Platform capability matrix
- Pin mapping tables
- Storage implementation details
- Transport layer architecture
- Limitations per platform
- Usage examples

**Estimated:** ~2,000 words (150 lines)

#### 10.2 Update PCF1_SPEC.md

**File:** `docs/PCF1_SPEC.md`

Add section on target-aware configuration:
- How validation checks platform capabilities
- Error messages for unsupported features
- Platform-specific configuration sections

**Lines Added:** ~100

#### 10.3 Session Tracking

**File:** `docs/tracking/2026-02-08__2247__zero-stub-portability.md`

All 7 required sections per AI_Instructions.md:
1. Session Summary
2. Pre-Flight Checks
3. Work Performed
4. Results
5. Build/Test Evidence
6. Failures/Variations
7. Next Actions

**Lines:** ~500

#### 10.4 Roadmap Update

**File:** `docs/roadmap.md`

Append-only entry for this session.

**Lines Added:** ~100

## Implementation Order

1. ✅ Phase 1: Structure (DONE)
2. ❌ Phase 2: Platform Packs (~1,800 lines)
3. ❌ Phase 3: I2C Transport (~300 lines)
4. ❌ Phase 4: SPI Transport (~450 lines)
5. ❌ Phase 5: UART Transport (~360 lines)
6. ❌ Phase 6: OneWire Transport (~450 lines)
7. ❌ Phase 7: Integration (~400 lines)
8. ❌ Phase 8: Build Config (~80 lines)
9. ❌ Phase 9: CI/CD (~60 lines)
10. ❌ Phase 10: Documentation (~750 lines)

**Total Lines:** ~4,650 lines of code + 2,000 words documentation

## Risk Assessment

### High Risk
- **Multi-platform compatibility:** Each platform has quirks that may require significant debugging
- **Hardware dependencies:** Full testing requires physical hardware for all 3 platforms
- **Library dependencies:** Different Arduino cores have different APIs

### Medium Risk
- **Build system complexity:** Managing 3 different build environments
- **Pin conflicts:** Platform-specific pin restrictions may cause issues
- **Storage differences:** LittleFS behavior differs between ESP8266 and RP2040

### Low Risk
- **CI/CD setup:** Well-understood GitHub Actions workflow
- **Documentation:** Straightforward technical writing

## Testing Strategy

### Unit Testing
Not practical without physical hardware. Rely on:
- Compilation as primary test
- Code review for correctness
- Runtime testing when hardware available

### Integration Testing
- Build for all 3 platforms
- Check binary sizes
- Verify no stub markers
- Manual CLI testing (when hardware available)

### Regression Testing
- Existing ESP32 functionality must continue to work
- BME280 driver should work on all platforms with I2C

## Success Metrics

1. **Build Success Rate:** 100% for all 3 platforms
2. **Code Coverage:** 0 stubs in platform/transport directories
3. **Binary Size:** Within reasonable limits (< 1MB for ESP32, < 500KB for ESP8266/RP2040)
4. **Capability Accuracy:** Platform Pack reports match reality
5. **Documentation Completeness:** All required docs present

## Next Steps

After this implementation:
1. Hardware validation on all 3 platforms
2. Add more device drivers leveraging transport layer
3. Performance optimization
4. Power management features
5. Advanced diagnostics

## Conclusion

This is a substantial implementation effort that will make PocketOS truly portable and production-ready. The key to success is:
1. Methodical approach (one phase at a time)
2. Extensive testing at each phase
3. Clear documentation
4. No shortcuts - real implementations only

**Estimated Timeline:** 2-3 weeks of focused development with proper hardware testing.
