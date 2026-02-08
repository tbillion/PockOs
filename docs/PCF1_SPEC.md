# PCF1 Specification
## PocketOS Configuration Format 1

**Version:** 1.0.0  
**Status:** Stable  
**Date:** 2026-02-08

---

## Overview

PCF1 (PocketOS Configuration Format 1) is a human-readable, line-oriented text format for complete system configuration. It supports validation, import/export, and persistence with round-trip integrity.

## Format Specification

### Structure

```
<file> ::= <section>*
<section> ::= <section_header> <line>*
<section_header> ::= '[' <section_name> ']' '\n'
<line> ::= <comment> | <key_value> | <blank_line>
<comment> ::= '#' <text> '\n'
<key_value> ::= <key> '=' <value> '\n'
<blank_line> ::= '\n'
```

### Sections

#### [system]
Core system information.

**Required keys:**
- `version` - PCF1 format version (e.g., "1.0.0")
- `platform` - Platform name (e.g., "esp32", "esp8266", "rp2040")

**Optional keys:**
- `chip` - Chip family (e.g., "ESP32-D0WD")
- `board` - Board name
- `flash_size` - Flash size in bytes

**Example:**
```
[system]
version=1.0.0
platform=esp32
chip=ESP32-D0WD
```

#### [hal]
Hardware abstraction layer capabilities.

**Keys:**
- `gpio_count` - Number of GPIO pins
- `adc_channels` - Number of ADC channels
- `pwm_channels` - Number of PWM channels
- `i2c_count` - Number of I2C controllers
- `spi_count` - Number of SPI controllers
- `uart_count` - Number of UART controllers

**Example:**
```
[hal]
gpio_count=40
adc_channels=18
pwm_channels=16
i2c_count=2
```

#### [i2cN]
I2C bus configuration (N = 0, 1, ...).

**Keys:**
- `sda` - SDA pin number
- `scl` - SCL pin number
- `speed_hz` - Bus speed in Hz (e.g., 100000, 400000)

**Validation:**
- Pin numbers must be valid for platform
- Speed must be > 0 and ≤ 1000000 Hz

**Example:**
```
[i2c0]
sda=21
scl=22
speed_hz=400000
```

#### [device:ID]
Device configuration (ID = numeric device ID).

**Required keys:**
- `endpoint` - Endpoint address (e.g., "gpio.dout.2", "i2c0:0x76")
- `driver` - Driver identifier (e.g., "gpio_dout", "bme280")
- `state` - Device state ("enabled", "disabled")

**Optional keys:**
- Driver-specific parameters

**Example:**
```
[device:1]
endpoint=i2c0:0x76
driver=bme280
state=enabled
mode=normal
oversample_temp=16x
oversample_press=16x
oversample_hum=16x
filter=16

[device:2]
endpoint=gpio.dout.2
driver=gpio_dout
state=enabled
initial_state=LOW
```

### Data Types

**Integer:** Decimal numbers (e.g., 21, 400000)  
**String:** Alphanumeric with underscores/hyphens (e.g., esp32, gpio_dout)  
**State:** Enumeration (enabled, disabled)  
**Endpoint:** Formatted address (see Endpoint Format)

### Endpoint Format

```
<endpoint> ::= <endpoint_type> '.' <qualifier> '.' <specifier>
             | <bus_endpoint> ':' <address>
             
<endpoint_type> ::= 'gpio' | 'adc' | 'pwm'
<qualifier> ::= 'din' | 'dout' | 'pwm' | 'ch'
<specifier> ::= <number>
<bus_endpoint> ::= 'i2c0' | 'i2c1' | 'spi0' | 'spi1'
<address> ::= '0x' <hex_number>
```

**Examples:**
- `gpio.dout.2` - GPIO digital output on pin 2
- `gpio.din.4` - GPIO digital input on pin 4
- `adc.ch.0` - ADC channel 0
- `pwm.ch.5` - PWM channel 5
- `i2c0:0x76` - I2C device at address 0x76 on bus 0

### Comments

Comments start with `#` and extend to end of line.
Comments are ignored during parsing.

**Example:**
```
# This is a comment
[system]
version=1.0.0  # Inline comment
```

### Blank Lines

Blank lines are allowed and ignored.
Used for readability between sections.

## Validation Rules

### Section Validation

**[system]:**
- `version` must match PCF1 version (currently "1.0.0")
- `platform` must be recognized (esp32, esp8266, rp2040)

**[hal]:**
- All count values must be ≥ 0
- Values should match platform capabilities

**[i2cN]:**
- Pin numbers must be valid for platform
- Pins must not conflict with flash/system pins
- Speed must be in valid range (1-1000000 Hz)
- SDA and SCL must be different pins

**[device:ID]:**
- Endpoint must exist or be registerable
- Driver must be available
- State must be "enabled" or "disabled"
- No duplicate device IDs
- No endpoint conflicts

### Value Validation

**Pin numbers:**
- Must be within platform's GPIO range
- Must not be reserved (flash, strapping)
- Checked against platform pack's `isSafePin()`

**I2C addresses:**
- Must be 7-bit addresses (0x08-0x77)
- Reserved addresses (0x00-0x07, 0x78-0x7F) rejected

**Device state:**
- Must be exactly "enabled" or "disabled"
- Case-sensitive

### Conflict Detection

**Resource conflicts:**
- GPIO pins cannot be used by multiple devices
- I2C/SPI buses cannot have conflicting configurations
- ADC/PWM channels must be exclusive

**Endpoint conflicts:**
- Same endpoint cannot be bound to multiple drivers
- Bus endpoints must be configured before device binding

## Import/Export Workflow

### Export

1. Call `PCF1Config::exportConfig()`
2. System iterates all configuration:
   - System info
   - HAL capabilities
   - Bus configurations
   - Device bindings
3. Generates PCF1-formatted text
4. Returns complete configuration string

### Import

1. Call `PCF1Config::importConfig(config)`
2. Validation phase:
   - Parse all lines
   - Validate sections and keys
   - Check for conflicts
   - Collect errors
3. If validation fails:
   - Return false
   - Errors available via `getValidationErrors()`
4. If validation succeeds:
   - Apply configuration
   - Configure buses
   - Bind devices
   - Set parameters
5. Return success status

### Validate Only

1. Call `PCF1Config::validateConfig(config)`
2. Performs validation without applying
3. Returns true/false
4. Errors available via `getValidationErrors()`

### Factory Reset

1. Call `PCF1Config::factoryReset()`
2. Unbind all devices
3. Clear persistence storage
4. Reset to defaults
5. Return success status

## Round-Trip Integrity

**Guarantee:** A valid configuration exported and then imported must restore the exact same system state.

**Test procedure:**
```
1. Configure system fully
2. Export: config1 = exportConfig()
3. Factory reset
4. Import: importConfig(config1)
5. Export: config2 = exportConfig()
6. Assert: config1 == config2 (ignoring comment/whitespace differences)
```

## Error Reporting

Validation errors are line-oriented with format:
```
Line <N>: <error_description>
```

**Example:**
```
Line 5: Invalid format
Line 12: Validation failed for i2c0.sda=99
Line 15: Endpoint conflict detected
```

## CLI Integration

### Export

```
> config export
[system]
version=1.0.0
platform=esp32
...
```

### Import

```
> config import [system]\nversion=1.0.0\n...
OK: Configuration imported
```

### Validate

```
> config validate [system]\nversion=2.0.0\n...
ERR_BAD_ARGS: Configuration validation failed
Line 2: Version mismatch
```

### Factory Reset

```
> factory_reset
OK: Factory reset complete
```

## Implementation Notes

### Memory Considerations

- Configuration stored as String (dynamic allocation)
- Consider chunked processing for large configs
- Limit maximum configuration size (e.g., 4KB)

### Platform-Specific Handling

- Validation rules adapt to platform capabilities
- Pin safety checks use platform pack
- Persistence mechanism varies by platform:
  - ESP32: NVS (flash-based key-value)
  - ESP8266: EEPROM or LittleFS
  - RP2040: Flash filesystem

### Future Extensions

**Possible v1.1 features:**
- Compressed format option
- Binary serialization
- Incremental updates
- Configuration templating
- Multi-file includes

**Backward compatibility:**
- Version field allows format evolution
- Unknown sections/keys are preserved
- Strict validation can be relaxed for forward compatibility

## Example Complete Configuration

```
# PocketOS Configuration
# Generated: 2026-02-08 22:30:00 UTC
# Platform: ESP32

[system]
version=1.0.0
platform=esp32
chip=ESP32-D0WD

[hal]
gpio_count=40
adc_channels=18
pwm_channels=16
i2c_count=2

[i2c0]
sda=21
scl=22
speed_hz=400000

[device:1]
endpoint=i2c0:0x76
driver=bme280
state=enabled
mode=normal
oversample_temp=16x
oversample_press=16x
oversample_hum=16x
filter=16

[device:2]
endpoint=gpio.dout.2
driver=gpio_dout
state=enabled
initial_state=LOW

# End of configuration
```

## Versioning

**Current version:** 1.0.0

**Version format:** MAJOR.MINOR.PATCH

- **MAJOR:** Breaking changes to format
- **MINOR:** New sections/keys (backward compatible)
- **PATCH:** Bug fixes, clarifications

**Compatibility promise:** Configurations with same MAJOR version must be importable. Future versions may add sections/keys but must not break existing valid configurations.

---

**Specification Version:** 1.0.0  
**Status:** Stable  
**Last Updated:** 2026-02-08
