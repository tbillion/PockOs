# PocketOS v1.0 - Manual Smoke Test Procedure

## Overview

This document provides the exact CLI smoke test procedure for PocketOS v1.0. Use this to verify the system is functioning correctly after flashing to ESP32.

---

## Prerequisites

- ESP32 board flashed with PocketOS v1.0
- USB connection to computer
- Serial terminal at 115200 baud (or `pio device monitor`)

---

## Expected Boot Output

Upon reset/power-on, you should see:

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
[INFO] Device bindings loaded
[INFO] CLI initialized
PocketOS Ready
> 
```

---

## Smoke Test Commands

Execute these commands in order. Each section tests a specific subsystem.

### Test 1: Help System
**Command:**
```
help
```

**Expected Output:**
```
PocketOS CLI Commands:
  help                           - Show this help
  sys info                       - System information
  hal caps                       - Hardware capabilities
  ep list                        - List endpoints
  ep probe <endpoint>            - Probe endpoint (e.g., i2c0)
  dev list                       - List devices
  bind <driver> <endpoint>       - Bind device (e.g., bind gpio.dout gpio.dout.2)
  unbind <device_id>             - Unbind device
  param get <dev_id> <param>     - Get parameter
  param set <dev_id> <param> <val> - Set parameter
  schema <device_id>             - Get device schema
  log tail [n]                   - Show last n log lines
  log clear                      - Clear log
  persist save                   - Save configuration
  persist load                   - Load configuration
```

**Pass Criteria:** All 15 commands listed

---

### Test 2: System Information (Intent: sys.info)
**Command:**
```
sys info
```

**Expected Output:**
```
version=1.0.0
board=ESP32
chip=ESP32
flash_size=4194304
heap_size=327680
free_heap=XXXXXX
```

**Pass Criteria:** 
- Board shows "ESP32"
- Flash size > 0
- Free heap > 0

---

### Test 3: Hardware Capabilities (Intent: hal.caps)
**Command:**
```
hal caps
```

**Expected Output:**
```
gpio_count=40
adc_count=18
pwm_count=16
i2c_count=2
spi_count=3
uart_count=3
```

**Pass Criteria:** 
- GPIO count = 40 (for ESP32)
- All capability counts shown

---

### Test 4: Endpoint Listing (Intent: ep.list)
**Command:**
```
ep list
```

**Expected Output:**
```
i2c0 (i2c.bus) [0]
```

**Pass Criteria:** 
- At least one endpoint listed
- I2C0 bus endpoint registered

---

### Test 5: Endpoint Probing (Intent: ep.probe)
**Command:**
```
ep probe i2c0
```

**Expected Output (if I2C enabled):**
```
I2C0 scan:
  0x48
  0x76
  [or "No devices found" if nothing connected]
```

**Expected Output (if I2C disabled):**
```
I2C not enabled
```

**Pass Criteria:** 
- Command executes without error
- Returns scan results or graceful message

---

### Test 6: Device Binding (Intent: dev.bind)
**Command:**
```
bind gpio.dout gpio.dout.2
```

**Expected Output:**
```
device_id=1
```

**Pass Criteria:** 
- Returns numeric device_id
- No error messages

---

### Test 7: Device Listing (Intent: dev.list)
**Command:**
```
dev list
```

**Expected Output:**
```
dev1: gpio.dout @ gpio.dout.2 [READY] fails:0
```

**Pass Criteria:** 
- Device 1 shown
- State is READY
- Endpoint is gpio.dout.2
- Fail count is 0

---

### Test 8: Parameter Set (Intent: param.set)
**Command:**
```
param set 1 state 1
```

**Expected Output:**
```
OK
```

**Pass Criteria:** 
- Returns "OK"
- If LED connected to GPIO2, it should turn on

---

### Test 9: Parameter Get (Intent: param.get)
**Command:**
```
param get 1 state
```

**Expected Output:**
```
state=1
```

**Pass Criteria:** 
- Returns current state
- Matches value set in Test 8

---

### Test 10: Toggle State
**Command:**
```
param set 1 state 0
```

**Expected Output:**
```
OK
```

**Pass Criteria:** 
- Returns "OK"
- If LED connected, it should turn off

---

### Test 11: Device Schema (Intent: schema.get)
**Command:**
```
schema 1
```

**Expected Output:**
```
[settings]
state:bool:rw:0.00-1.00
pin:int:ro

[signals]
output:bool:ro

[commands]
toggle
```

**Pass Criteria:** 
- Shows settings section with "state" parameter
- Shows signals section
- Shows commands section
- Format is line-oriented (not JSON)

---

### Test 12: Log Tail (Intent: log.tail)
**Command:**
```
log tail 5
```

**Expected Output:**
```
[INFO] GPIO DOUT driver initialized on pin 2
[INFO] Device 1 bound to gpio.dout.2
[INFO] PocketOS Ready
```

**Pass Criteria:** 
- Shows recent log entries
- Log entries have [INFO], [WARN], or [ERROR] prefix

---

### Test 13: Persistence Save
**Command:**
```
persist save
```

**Expected Output:**
```
OK
```

**Pass Criteria:** 
- Returns "OK"
- No error messages

---

### Test 14: Device Unbind
**Command:**
```
unbind 1
```

**Expected Output:**
```
OK
```

**Pass Criteria:** 
- Returns "OK"
- Device 1 removed from registry

---

### Test 15: Persistence Load
**Command:**
```
persist load
```

**Expected Output:**
```
OK
```

**Pass Criteria:** 
- Returns "OK"
- Device 1 should be restored (check with `dev list`)

---

## Complete Test Sequence Script

Copy and paste this entire sequence into the serial terminal:

```
help
sys info
hal caps
ep list
ep probe i2c0
bind gpio.dout gpio.dout.2
dev list
param set 1 state 1
param get 1 state
schema 1
param set 1 state 0
log tail 5
persist save
unbind 1
dev list
persist load
dev list
```

---

## Pass/Fail Criteria

### PASS: Smoke Test Successful
- All 15 tests execute without errors
- Intent API dispatcher routes all commands correctly
- Device binding/unbinding works
- Parameter get/set functions
- Schema system returns driver metadata
- Logging captures events
- Persistence saves and restores state

### FAIL: Smoke Test Failed
If any of these occur:
- Command not recognized
- Intent returns ERR_INTERNAL
- Device binding fails with ERR_CONFLICT when it shouldn't
- Parameter operations fail
- Schema missing or malformed
- Persistence doesn't save/restore devices

---

## Troubleshooting

### Command Not Recognized
- **Issue:** CLI parser not mapping command to intent
- **Check:** CLI::parseCommand() in src/pocketos/cli/cli.cpp

### Intent Returns Error
- **Issue:** Intent handler failing
- **Check:** IntentAPI::dispatch() and specific handler
- **Check:** Log output with `log tail`

### Device Binding Fails
- **Issue:** Resource manager conflict or endpoint not found
- **Check:** `ep list` to verify endpoint exists
- **Check:** Resource manager claims with debug output

### Parameter Operations Fail
- **Issue:** Driver not implementing setParam/getParam correctly
- **Check:** Driver implementation in src/pocketos/drivers/

### Schema Not Returned
- **Issue:** Driver getSchema() not implemented
- **Check:** IDriver interface implementation

---

## Success Indicators

After successful smoke test, you should be able to:

1. ✅ Execute all 15 CLI commands
2. ✅ Bind drivers to hardware endpoints
3. ✅ Control hardware via parameters (e.g., toggle GPIO)
4. ✅ Query device schemas
5. ✅ View system logs
6. ✅ Save and restore device configurations
7. ✅ See consistent state across operations

---

## Vocabulary Verification

The smoke test also validates the three vocabularies:

### 1. Capability Vocabulary
- **Test:** `schema 1`
- **Validates:** Settings (bool/int), signals, commands
- **Format:** Line-oriented, typed, with constraints

### 2. Transport/Bus Vocabulary
- **Test:** `ep list`, `ep probe i2c0`
- **Validates:** Endpoint types (gpio.dout, i2c0), addressing
- **Format:** Typed endpoints with resource IDs

### 3. Control Vocabulary
- **Test:** All commands
- **Validates:** 17 intent opcodes, 7 error codes
- **Format:** Intent API v1.0.0 with line-oriented responses

---

## Reporting Results

Document your smoke test results in this format:

```
PocketOS v1.0 Smoke Test Report
Date: YYYY-MM-DD
Hardware: ESP32-DevKitC / [your board]
Build: [commit hash]

Results:
- Test 1 (help): PASS/FAIL
- Test 2 (sys info): PASS/FAIL
- Test 3 (hal caps): PASS/FAIL
[... continue for all 15 tests]

Overall: PASS/FAIL

Notes:
[Any observations or issues]
```

---

**End of Manual Smoke Test Procedure**

For automated testing, use `build_and_test.sh` script.
For build issues, see `docs/BUILD_TROUBLESHOOTING.md`.
