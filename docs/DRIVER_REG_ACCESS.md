# PocketOS Register Access CLI Guide

## Overview

This document demonstrates CLI usage for accessing device registers directly through the PocketOS register access interface. Register access is only available for Tier 2 drivers.

## Prerequisites

- Device driver must be at Tier 2 (compiled with `POCKETOS_DRIVER_TIER=2` or `POCKETOS_DRIVER_TIER_<DRIVER>=2`)
- Device must be bound to an endpoint
- Driver must implement full register map

## Build Configuration

To enable register access, compile with Tier 2:

```bash
# Global Tier 2 (all drivers)
pio run -e esp32dev-full

# Or use build flag
pio run -e esp32dev --build-flags="-DPOCKETOS_DRIVER_TIER=2"

# Per-driver override (BME280 only to Tier 2)
pio run -e esp32dev --build-flags="-DPOCKETOS_DRIVER_TIER_BME280=2"
```

## Complete Workflow Example: BME280

### Step 1: System Initialization

```
=====================================
       PocketOS v1.0
  Embedded OS for Microcontrollers
=====================================

[INFO] Logger initialized
[INFO] HAL initialized
[INFO] Intent API v1.0.0 initialized
[INFO] Resource Manager initialized
[INFO] Endpoint Registry initialized
[INFO] Device Registry initialized
[INFO] Persistence initialized
[INFO] CLI initialized
PocketOS Ready
> 
```

### Step 2: List Available Endpoints

```
> ep list
i2c0 [READY]
i2c1 [READY]
gpio.dout.0 [READY]
gpio.dout.1 [READY]
...
OK
```

### Step 3: Probe I2C Bus

```
> ep probe i2c0
Scanning I2C bus i2c0...
Found device at 0x76
Found device at 0x77
Scan complete. Found 2 devices.
OK
```

### Step 4: Bind BME280 Driver

```
> bind bme280 i2c0:0x76
[INFO] BME280: Initializing at address 0x76
[INFO] BME280: Initialized successfully
device_id=1
OK
```

### Step 5: Verify Device Status

```
> status 1
device_id=1
endpoint=i2c0:0x76
driver=bme280
state=READY
init_failures=0
io_failures=0
last_ok_ms=5234
uptime_ms=123
OK
```

### Step 6: Read Sensor Data (Verify Driver Works)

```
> read 1
temperature=22.5°C
humidity=45.2%RH
pressure=1013.25hPa
OK
```

### Step 7: List All Registers

```
> reg list 1
0x88 DIG_T1_LSB 1 RO 0x00
0x89 DIG_T1_MSB 1 RO 0x00
0x8A DIG_T2_LSB 1 RO 0x00
0x8B DIG_T2_MSB 1 RO 0x00
0x8C DIG_T3_LSB 1 RO 0x00
0x8D DIG_T3_MSB 1 RO 0x00
0x8E DIG_P1_LSB 1 RO 0x00
0x8F DIG_P1_MSB 1 RO 0x00
0x90 DIG_P2_LSB 1 RO 0x00
0x91 DIG_P2_MSB 1 RO 0x00
0x92 DIG_P3_LSB 1 RO 0x00
0x93 DIG_P3_MSB 1 RO 0x00
0x94 DIG_P4_LSB 1 RO 0x00
0x95 DIG_P4_MSB 1 RO 0x00
0x96 DIG_P5_LSB 1 RO 0x00
0x97 DIG_P5_MSB 1 RO 0x00
0x98 DIG_P6_LSB 1 RO 0x00
0x99 DIG_P6_MSB 1 RO 0x00
0x9A DIG_P7_LSB 1 RO 0x00
0x9B DIG_P7_MSB 1 RO 0x00
0x9C DIG_P8_LSB 1 RO 0x00
0x9D DIG_P8_MSB 1 RO 0x00
0x9E DIG_P9_LSB 1 RO 0x00
0x9F DIG_P9_MSB 1 RO 0x00
0xA1 DIG_H1 1 RO 0x00
0xD0 CHIP_ID 1 RO 0x60
0xE0 RESET 1 WO 0x00
0xE1 DIG_H2_LSB 1 RO 0x00
0xE2 DIG_H2_MSB 1 RO 0x00
0xE3 DIG_H3 1 RO 0x00
0xE4 DIG_H4_MSB 1 RO 0x00
0xE5 DIG_H4_LSB_H5_MSB 1 RO 0x00
0xE6 DIG_H5_LSB 1 RO 0x00
0xE7 DIG_H6 1 RO 0x00
0xF2 CTRL_HUM 1 RW 0x00
0xF3 STATUS 1 RO 0x00
0xF4 CTRL_MEAS 1 RW 0x00
0xF5 CONFIG 1 RW 0x00
0xF7 PRESS_MSB 1 RO 0x80
0xF8 PRESS_LSB 1 RO 0x00
0xF9 PRESS_XLSB 1 RO 0x00
0xFA TEMP_MSB 1 RO 0x80
0xFB TEMP_LSB 1 RO 0x00
0xFC TEMP_XLSB 1 RO 0x00
0xFD HUM_MSB 1 RO 0x80
0xFE HUM_LSB 1 RO 0x00
OK
```

Register list format: `<addr> <name> <width> <access> <reset_value>`
- `addr`: Hexadecimal register address
- `name`: CLI-friendly register name from datasheet
- `width`: Width in bytes (1, 2, 3, or 4)
- `access`: RO (read-only), WO (write-only), RW (read-write), RC (read-clear)
- `reset_value`: Default value after reset (hex)

### Step 8: Read Chip ID Register (by Address)

```
> reg read 1 0xD0
register=0xD0
value=60
length=1
OK
```

The value `0x60` is the BME280 chip ID.

### Step 9: Read Chip ID Register (by Name)

```
> reg read 1 CHIP_ID
register=0xD0
value=60
length=1
OK
```

Register names are case-insensitive.

### Step 10: Read Control Register

```
> reg read 1 CTRL_MEAS
register=0xF4
value=27
length=1
OK
```

Value `0x27` indicates:
- Temperature oversampling: 1x
- Pressure oversampling: 1x
- Mode: Normal

### Step 11: Read Status Register

```
> reg read 1 STATUS
register=0xF3
value=00
length=1
OK
```

Value `0x00` indicates:
- Not measuring
- Not updating

### Step 12: Read Multiple Registers (Sequential)

```
> reg read 1 0xF7 6
register=0xF7
value=4F:D8:00:7F:A5:00
length=6
OK
```

This reads 6 bytes starting from PRESS_MSB:
- Bytes 0-2: Pressure data (0x4FD800 >> 4 = raw pressure)
- Bytes 3-5: Temperature data (0x7FA500 >> 4 = raw temperature)

### Step 13: Modify Configuration (Write Register)

```
> reg write 1 CTRL_HUM 0x01
register=0xF2
value=0x01
OK
```

Set humidity oversampling to 1x.

### Step 14: Verify Write

```
> reg read 1 CTRL_HUM
register=0xF2
value=01
length=1
OK
```

Confirmed write succeeded.

### Step 15: Try Writing Read-Only Register (Should Fail)

```
> reg write 1 CHIP_ID 0xFF
Error: ERR_IO - Failed to write register (may be read-only)
```

Register access validation prevents writes to read-only registers.

### Step 16: Try Reading Write-Only Register (Should Fail)

```
> reg read 1 RESET
Error: ERR_IO - Failed to read register
```

Register access validation prevents reads from write-only registers.

## Error Scenarios

### Device Not Found

```
> reg list 99
Error: ERR_NOT_FOUND - Device not found
```

### Device Not at Tier 2

```
> bind gpio.dout gpio.dout.2
device_id=2
OK

> reg list 2
Error: ERR_UNSUPPORTED - Device does not support register access. Enable POCKETOS_DRIVER_TIER=2 and use Tier 2 driver.
```

GPIO drivers don't have register maps.

### Invalid Register Address

```
> reg read 1 0xFF
Error: ERR_IO - Failed to read register
```

Register doesn't exist in the device.

### Invalid Syntax

```
> reg read 1
Error: ERR_BAD_ARGS - Usage: reg.read <device_id> <reg|name> [len]
```

## Advanced Usage

### Hex vs. Decimal Addressing

Both work:
```
> reg read 1 0xD0    # Hexadecimal (recommended)
> reg read 1 208     # Decimal (same address)
```

### Case-Insensitive Names

All work:
```
> reg read 1 CHIP_ID
> reg read 1 chip_id
> reg read 1 Chip_Id
```

### Reading Raw Sensor Data

```
# Read raw pressure (3 bytes)
> reg read 1 PRESS_MSB 3
register=0xF7
value=4F:D8:00
length=3
OK

# Convert: (0x4FD800 >> 4) = 0x4FD8 = 20440 (raw ADC value)
# Apply compensation formula to get hPa
```

### Writing Measurement Mode

```
# Force BME280 into forced mode
> reg write 1 CTRL_MEAS 0x25
register=0xF4
value=0x25
OK

# Value breakdown:
# Bits 7-5: Temperature oversampling = 001 (1x)
# Bits 4-2: Pressure oversampling = 001 (1x)
# Bits 1-0: Mode = 01 (forced mode)
```

### Soft Reset via Register

```
> reg write 1 RESET 0xB6
register=0xE0
value=0xB6
OK

# BME280 will reset and reinitialize
# Wait a moment before next access
```

## Integration with Scripts

Register access can be scripted:

```bash
#!/bin/bash
# Read BME280 chip ID
echo "reg read 1 0xD0" | picocom -b 115200 /dev/ttyUSB0

# Configure sensor
echo "reg write 1 0xF2 0x01" | picocom -b 115200 /dev/ttyUSB0
echo "reg write 1 0xF4 0x27" | picocom -b 115200 /dev/ttyUSB0
```

## Register Access vs. High-Level API

### When to Use Register Access
- Debugging sensor issues
- Testing new configurations
- Implementing features not yet in driver
- Hardware validation and testing
- Learning device behavior

### When to Use High-Level API
- Normal application usage
- Reading compensated sensor values
- Production code
- Reliable, tested operations

Example:
```
# High-level (preferred for apps)
> read 1
temperature=22.5°C
humidity=45.2%RH
pressure=1013.25hPa

# Low-level (debugging/testing)
> reg read 1 TEMP_MSB 3
register=0xFA
value=7F:A5:00
# Then manually apply compensation formulas
```

## Supported Drivers

Currently Tier 2 drivers with full register access:

| Driver | Addresses | Register Count | Notes |
|--------|-----------|----------------|-------|
| BME280 | 0x76, 0x77 | 45 | Temperature, humidity, pressure sensor |

## Future Enhancements

Planned features:
- Register name aliases (e.g., "0xD0" and "ID" both work)
- Batch register operations
- Register monitoring/watching
- Register diff between reads
- Export register state to file
- Import register configuration from file

## References

- **BME280 Datasheet**: Bosch BME280 Combined humidity and pressure sensor (BST-BME280-DS002)
- **Driver Authoring Guide**: `docs/DRIVER_AUTHORING_GUIDE.md`
- **Driver Tiering**: `docs/DRIVER_TIERING.md`
- **Intent API**: `src/pocketos/core/intent_api.h`
