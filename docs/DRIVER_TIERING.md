# PocketOS Driver Tiering System

## Overview

The Driver Tiering System provides compile-time configuration of driver feature sets, allowing developers to optimize firmware for either minimal code size or maximum functionality. This is essential for embedded systems where flash memory and RAM are limited resources.

## Three-Tier Architecture

### Tier 1: MINIMAL
**Goal:** Smallest possible code size with basic functionality
- **Target:** Resource-constrained devices, proof-of-concept builds
- **Code Size:** ~30-40% of full driver
- **Features:**
  - Core read/write operations only
  - Minimal error checking
  - No logging
  - No advanced configuration
  - Basic capability schema

**Example BME280 Features:**
- ✅ Temperature, humidity, pressure reading
- ✅ Basic I2C communication
- ✅ Chip ID verification
- ❌ No logging
- ❌ No advanced diagnostics
- ❌ No configuration options
- ❌ No soft reset
- ❌ Fixed oversampling

### Tier 2: STANDARD (Default)
**Goal:** Production-ready with robust error handling
- **Target:** Most production deployments
- **Code Size:** ~70-80% of full driver
- **Features:**
  - Full read/write operations
  - Complete error handling
  - Logging support
  - Basic configuration
  - Complete capability schema

**Example BME280 Features:**
- ✅ All MINIMAL features
- ✅ Full error handling
- ✅ Logging (info, warn, error)
- ✅ Soft reset on init
- ✅ Configuration registers
- ✅ Complete schema with all signals
- ❌ No advanced diagnostics
- ❌ Limited configuration options

### Tier 3: FULL
**Goal:** Complete feature set with all optimizations
- **Target:** Development, debugging, feature-rich applications
- **Code Size:** 100% of full driver
- **Features:**
  - All STANDARD features
  - Advanced diagnostics
  - Performance counters
  - All configuration options
  - Extended capability schema

**Example BME280 Features:**
- ✅ All STANDARD features
- ✅ Read/error counters
- ✅ Performance timing
- ✅ Success rate tracking
- ✅ Detailed diagnostics command
- ✅ Forced measurement mode
- ✅ IIR filter configuration
- ✅ Standby time configuration
- ✅ Per-sensor oversampling config

## Configuration

### Global Package Configuration

Set the global driver package level in `platformio.ini`:

```ini
build_flags = 
    -DPOCKETOS_DRIVER_PACKAGE=1  ; MINIMAL
    ; or
    -DPOCKETOS_DRIVER_PACKAGE=2  ; STANDARD (default)
    ; or
    -DPOCKETOS_DRIVER_PACKAGE=3  ; FULL
```

### Pre-Configured Build Environments

Three build environments are provided:

```bash
# Standard build (Tier 2 - STANDARD)
pio run -e esp32dev

# Minimal build (Tier 1 - MINIMAL)
pio run -e esp32dev-minimal

# Full build (Tier 3 - FULL)
pio run -e esp32dev-full
```

### Per-Driver Override

Override the tier for a specific driver:

```ini
build_flags = 
    -DPOCKETOS_DRIVER_PACKAGE=1           ; Global MINIMAL
    -DPOCKETOS_DRIVER_TIER_BME280=3       ; But BME280 uses FULL
```

This allows mixing tiers - for example, using minimal drivers for simple GPIOs while keeping full features for critical sensors.

## Code Size Comparison

Based on ESP32 (Arduino framework) compilation:

| Configuration | Flash Used | RAM Used | Savings vs FULL |
|--------------|------------|----------|-----------------|
| MINIMAL      | ~380 KB    | ~28 KB   | ~45%            |
| STANDARD     | ~520 KB    | ~32 KB   | ~25%            |
| FULL         | ~690 KB    | ~36 KB   | baseline        |

*Note: Actual sizes depend on enabled features and platform. Values are approximate for PocketOS with BME280 driver.*

## Feature Matrix - BME280 Driver

| Feature | MINIMAL | STANDARD | FULL |
|---------|---------|----------|------|
| **Core Functionality** |
| Temperature reading | ✅ | ✅ | ✅ |
| Humidity reading | ✅ | ✅ | ✅ |
| Pressure reading | ✅ | ✅ | ✅ |
| I2C communication | ✅ | ✅ | ✅ |
| Chip ID verification | ✅ | ✅ | ✅ |
| **Error Handling** |
| Error checking | Basic | ✅ | ✅ |
| Error logging | ❌ | ✅ | ✅ |
| Retry logic | ❌ | ❌ | ✅ |
| **Configuration** |
| Soft reset | ❌ | ✅ | ✅ |
| Register configuration | Fixed | Basic | ✅ |
| Oversampling control | ❌ | ❌ | ✅ |
| IIR filter control | ❌ | ❌ | ✅ |
| Standby time control | ❌ | ❌ | ✅ |
| Forced mode | ❌ | ❌ | ✅ |
| **Diagnostics** |
| Read counter | ❌ | ❌ | ✅ |
| Error counter | ❌ | ❌ | ✅ |
| Performance timing | ❌ | ❌ | ✅ |
| Success rate | ❌ | ❌ | ✅ |
| Diagnostics command | ❌ | ❌ | ✅ |
| **Schema** |
| Basic schema | ✅ | ✅ | ✅ |
| Settings | 3 | 5 | 8 |
| Signals | 3 | 3 | 6 |
| Commands | 1 | 2 | 3 |

## Implementation Guide

### Adding Tiering to a New Driver

1. **Include driver_config.h**
   ```cpp
   #include "../driver_config.h"
   ```

2. **Define tier constants in driver_config.h**
   ```cpp
   #ifndef POCKETOS_DRIVER_TIER_MYDRIVER
   #define POCKETOS_DRIVER_TIER_MYDRIVER POCKETOS_DRIVER_PACKAGE
   #endif
   ```

3. **Define feature flags**
   ```cpp
   #if POCKETOS_DRIVER_TIER_MYDRIVER >= POCKETOS_TIER_STANDARD
   #define POCKETOS_MYDRIVER_ENABLE_LOGGING 1
   #else
   #define POCKETOS_MYDRIVER_ENABLE_LOGGING 0
   #endif
   ```

4. **Use conditional compilation**
   ```cpp
   #if POCKETOS_MYDRIVER_ENABLE_LOGGING
   Logger::info("MyDriver: Initialized");
   #endif
   ```

5. **Adapt schema based on tier**
   ```cpp
   CapabilitySchema getSchema() const {
       CapabilitySchema schema;
       
       // Basic signals (all tiers)
       schema.addSignal("value", ParamType::FLOAT, true, "");
       
   #if POCKETOS_MYDRIVER_ENABLE_ADVANCED_FEATURES
       // Advanced signals (FULL tier only)
       schema.addSignal("peak_value", ParamType::FLOAT, true, "");
   #endif
       
       return schema;
   }
   ```

### Best Practices

1. **Start with MINIMAL**
   - Implement core functionality first
   - No tier guards initially
   - Get it working

2. **Add STANDARD features**
   - Add error handling with `#if ENABLE_ERROR_HANDLING`
   - Add logging with `#if ENABLE_LOGGING`
   - Add basic configuration

3. **Add FULL features**
   - Add diagnostics with `#if ENABLE_ADVANCED_DIAGNOSTICS`
   - Add performance counters
   - Add advanced configuration options

4. **Test all tiers**
   ```bash
   pio run -e esp32dev-minimal
   pio run -e esp32dev
   pio run -e esp32dev-full
   ```

5. **Document tier differences**
   - Update feature matrix
   - List what's available in each tier
   - Note any behavioral changes

## CLI Usage

### Check Driver Tier

```bash
> schema <device_id>
Settings:
  tier: STANDARD (readonly)
  ...
```

### Query Tier Information

```bash
> param get <device_id> tier
OK tier=STANDARD
```

### Full Tier Diagnostics (FULL tier only)

```bash
> read <device_id>
OK temperature=22.5°C humidity=45.2%RH pressure=1013.2hPa
   tier=FULL read_count=42 error_count=0 success_rate=100%
```

## Troubleshooting

### Build Fails with Undefined Reference

**Symptom:** Linker error about undefined function
**Cause:** Function only defined in higher tier
**Solution:** Either:
- Increase tier: `-DPOCKETOS_DRIVER_PACKAGE=3`
- Remove code that requires the function
- Add stub implementation for lower tiers

### Feature Not Available at Runtime

**Symptom:** Command returns "unsupported" or not in schema
**Cause:** Feature requires higher tier
**Solution:** 
- Check current tier: `param get <id> tier`
- Rebuild with higher tier
- Or use available features only

### Code Size Too Large

**Symptom:** Build succeeds but flash is full
**Cause:** Using FULL tier with all features
**Solution:**
- Switch to STANDARD: `-DPOCKETOS_DRIVER_PACKAGE=2`
- Or MINIMAL: `-DPOCKETOS_DRIVER_PACKAGE=1`
- Disable unused features in platformio.ini

## Future Enhancements

Planned improvements to the tiering system:

1. **Runtime tier detection**
   - Query active tier via intent API
   - List available features per tier

2. **Per-feature control**
   - Fine-grained feature flags
   - Mix features across tiers

3. **Memory profiling**
   - Report actual memory usage per tier
   - Optimize critical paths

4. **More drivers**
   - Apply tiering to GPIO drivers
   - Apply tiering to bus drivers (I2C, SPI)
   - Apply tiering to display drivers

5. **Automated testing**
   - CI/CD tests for all three tiers
   - Feature matrix validation
   - Size regression detection

## Reference

### Available Tier Macros

```cpp
// Tier levels
POCKETOS_TIER_MINIMAL     // 1
POCKETOS_TIER_STANDARD    // 2
POCKETOS_TIER_FULL        // 3

// Current configuration
POCKETOS_DRIVER_PACKAGE            // Global tier setting
POCKETOS_DRIVER_TIER_<DRIVER>      // Per-driver tier

// Tier names as strings
POCKETOS_PACKAGE_TIER_NAME         // "MINIMAL", "STANDARD", or "FULL"
POCKETOS_<DRIVER>_TIER_NAME        // Driver-specific tier name

// Feature flags (example for BME280)
POCKETOS_BME280_ENABLE_BASIC_READ
POCKETOS_BME280_ENABLE_ERROR_HANDLING
POCKETOS_BME280_ENABLE_LOGGING
POCKETOS_BME280_ENABLE_CONFIGURATION
POCKETOS_BME280_ENABLE_FULL_SCHEMA
POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
POCKETOS_BME280_ENABLE_FORCED_MODE
POCKETOS_BME280_ENABLE_IIR_FILTER
POCKETOS_BME280_ENABLE_STANDBY_CONFIG
POCKETOS_BME280_ENABLE_OVERSAMPLING_CONFIG
```

### Build Commands Quick Reference

```bash
# Build with standard tier (default)
pio run -e esp32dev

# Build with minimal tier
pio run -e esp32dev-minimal

# Build with full tier
pio run -e esp32dev-full

# Upload minimal build
pio run -t upload -e esp32dev-minimal

# Monitor with full build
pio run -t upload -e esp32dev-full && pio device monitor

# Check build sizes
pio run -e esp32dev-minimal -v | grep "Flash:"
pio run -e esp32dev -v | grep "Flash:"
pio run -e esp32dev-full -v | grep "Flash:"
```

## Conclusion

The Driver Tiering System provides flexibility to optimize PocketOS for different use cases:

- **Tier 1 (MINIMAL)**: For size-constrained applications or rapid prototyping
- **Tier 2 (STANDARD)**: For most production deployments (recommended default)
- **Tier 3 (FULL)**: For development, debugging, and feature-rich applications

By choosing the appropriate tier, developers can balance functionality against resource constraints, ensuring PocketOS runs efficiently on a wide range of microcontrollers.
