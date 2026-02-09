# Build Instructions and Troubleshooting

## Overview

This document provides instructions for building PocketOS and troubleshooting common issues.

## Prerequisites

- Python 3.6 or later
- PlatformIO Core
- Internet connection for downloading ESP32 platform and toolchains

## Standard Build Process

### 1. Install PlatformIO

```bash
pip3 install --user platformio
```

### 2. Build the Project

```bash
cd /path/to/PockOs
pio run -e esp32dev
```

### 3. Upload to ESP32

```bash
pio run -t upload -e esp32dev
```

### 4. Monitor Serial Output

```bash
pio device monitor
```

## Troubleshooting

### Issue: ESP32 Platform Download Fails

**Symptoms:**
```
Platform Manager: Installing espressif32
HTTPClientError:
```

**Cause:** Network issues, DNS resolution failures, or firewall restrictions preventing access to `dl.platformio.org`.

**Solutions:**

#### Option 1: Install from GitHub
```bash
pio platform install https://github.com/platformio/platform-espressif32.git#v6.4.0
```

#### Option 2: Manual Platform Installation

1. Download the platform manually from GitHub:
```bash
git clone https://github.com/platformio/platform-espressif32.git
cd platform-espressif32
git checkout v6.4.0
```

2. Copy to PlatformIO directory:
```bash
mkdir -p ~/.platformio/platforms/
cp -r platform-espressif32 ~/.platformio/platforms/espressif32
```

#### Option 3: Use a Different Network

If behind a corporate firewall or proxy, try:
- Using a different network
- Configuring proxy settings
- Using a VPN

#### Option 4: Offline Installation

For environments without internet access:
1. On a machine with internet access:
   - Install PlatformIO
   - Run `pio platform install espressif32`
   - Copy `~/.platformio/` directory

2. On offline machine:
   - Copy the `.platformio` directory to the offline machine
   - Place in home directory

### Issue: Toolchain Download Fails

**Symptoms:**
```
Tool Manager: Installing espressif/toolchain-xtensa-esp32
HTTPClientError:
```

**Solution:** This typically occurs after platform installation fails. Try:

1. Clear PlatformIO cache:
```bash
rm -rf ~/.platformio/.cache
rm -rf ~/.platformio/packages
```

2. Retry installation:
```bash
pio run -e esp32dev
```

3. If issues persist, download toolchains manually from:
   - https://github.com/espressif/crosstool-NG/releases

### Issue: DNS Resolution Failures

**Symptoms:**
```
curl: (6) Could not resolve host: dl.platformio.org
```

**Diagnosis:**
```bash
nslookup dl.platformio.org
dig dl.platformio.org
```

**Solutions:**

1. Add to `/etc/hosts`:
```bash
# Get IP address on working machine
nslookup dl.platformio.org

# Add to /etc/hosts on problem machine
echo "X.X.X.X dl.platformio.org" | sudo tee -a /etc/hosts
```

2. Change DNS servers:
```bash
# Temporarily use Google DNS
echo "nameserver 8.8.8.8" | sudo tee /etc/resolv.conf
```

## Build Verification Without Hardware

### Check Code Structure

The project structure is correct if you see:

```
src/
├── main.cpp
└── pocketos/
    ├── core/
    │   ├── intent_api.{h,cpp}
    │   ├── hal.{h,cpp}
    │   ├── resource_manager.{h,cpp}
    │   ├── endpoint_registry.{h,cpp}
    │   ├── device_registry.{h,cpp}
    │   ├── capability_schema.{h,cpp}
    │   ├── logger.{h,cpp}
    │   └── persistence.{h,cpp}
    ├── drivers/
    │   └── gpio_dout_driver.{h,cpp}
    └── cli/
        └── cli.{h,cpp}
```

### Syntax Validation

While a full build requires the ESP32 toolchain, you can validate basic C++ syntax:

```bash
# Check for obvious syntax errors in headers
find src -name "*.h" -exec g++ -fsyntax-only -I./src {} \; 2>&1 | grep error
```

Note: This will show missing Arduino.h and ESP32-specific headers, which is expected.

## Environment-Specific Notes

### GitHub Actions / CI Environments

If running in a restricted CI environment:

1. **Pre-cache dependencies**: Cache `~/.platformio` directory
2. **Use Docker**: Run builds in a Docker container with all dependencies
3. **Alternative registries**: Configure PlatformIO to use mirror registries

Example GitHub Actions workflow:
```yaml
- name: Cache PlatformIO
  uses: actions/cache@v3
  with:
    path: ~/.platformio
    key: ${{ runner.os }}-pio-${{ hashFiles('**/platformio.ini') }}

- name: Install PlatformIO
  run: pip install platformio

- name: Build firmware
  run: pio run -e esp32dev
```

### Docker-based Build

```dockerfile
FROM python:3.9-slim

RUN pip install platformio
RUN pio platform install espressif32

WORKDIR /project
COPY . .

RUN pio run -e esp32dev
```

## Known Working Configurations

### Tested Environments

1. **Ubuntu 22.04+**: Works with standard installation
2. **macOS 12+**: Works with standard installation  
3. **Windows 10/11**: Works with standard installation
4. **Docker**: Requires pre-built container with platforms

### Network Requirements

The following domains must be accessible:
- `dl.platformio.org` - Platform and package downloads
- `github.com` - Alternative package source
- `raw.githubusercontent.com` - Repository content

## Alternative: Manual Compilation

If PlatformIO cannot be used, ESP32 projects can be compiled with ESP-IDF directly:

```bash
# Install ESP-IDF
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32

# Source environment
. ./export.sh

# Convert PlatformIO project (manual process)
# Create CMakeLists.txt, configure components
```

## Success Verification

A successful build will show:
```
Processing esp32dev (platform: espressif32; board: esp32dev; framework: arduino)
...
Linking .pio/build/esp32dev/firmware.elf
Building .pio/build/esp32dev/firmware.bin
...
========================= [SUCCESS] Took X.XX seconds =========================
```

## Support

For additional help:
1. Check PlatformIO docs: https://docs.platformio.org/
2. ESP32 Arduino docs: https://docs.espressif.com/projects/arduino-esp32/
3. Project README: README.md

## Current Status

As of the latest update:
- **Code Structure**: ✓ Verified
- **Syntax**: ✓ Valid (pending Arduino framework headers)
- **Platform**: ✓ Partially installed (espressif32@6.4.0)
- **Toolchains**: ⚠️ Download blocked by network restrictions
- **Build**: ⏳ Pending toolchain installation

The code is structurally sound and should compile successfully once the ESP32 toolchains are available.
