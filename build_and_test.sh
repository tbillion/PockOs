#!/bin/bash
# PocketOS v1.0 - Build and Smoke Test Script
# This script performs a complete build and CLI smoke test

set -e

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║                PocketOS v1.0 Build & Smoke Test              ║"
echo "╔═══════════════════════════════════════════════════════════════╗"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

echo "Project Directory: $PROJECT_DIR"
echo ""

# Step 1: Check PlatformIO
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 1: Checking PlatformIO Installation"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if ! command -v pio &> /dev/null; then
    echo -e "${RED}✗ PlatformIO not found${NC}"
    echo ""
    echo "Install PlatformIO:"
    echo "  pip3 install platformio"
    echo "or"
    echo "  python3 -m pip install platformio"
    exit 1
fi

echo -e "${GREEN}✓ PlatformIO found: $(pio --version)${NC}"
echo ""

# Step 2: Build firmware
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 2: Building Firmware"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "Command: pio run -e esp32dev"
echo ""

if pio run -e esp32dev; then
    echo -e "${GREEN}✓ Build successful${NC}"
    BUILD_SUCCESS=true
else
    echo -e "${RED}✗ Build failed${NC}"
    echo ""
    echo "If you see HTTPClientError or network issues:"
    echo "  1. Check internet connection"
    echo "  2. See docs/BUILD_TROUBLESHOOTING.md for solutions"
    echo "  3. Try: pio platform install https://github.com/platformio/platform-espressif32.git#v6.4.0"
    exit 1
fi

echo ""

# Step 3: Flash to ESP32
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 3: Flashing to ESP32"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "Connect ESP32 board via USB and press Enter..."
read -p ""

echo "Command: pio run -t upload -e esp32dev"
echo ""

if pio run -t upload -e esp32dev; then
    echo -e "${GREEN}✓ Flash successful${NC}"
else
    echo -e "${RED}✗ Flash failed${NC}"
    echo ""
    echo "Common issues:"
    echo "  - ESP32 not connected"
    echo "  - Wrong USB port selected"
    echo "  - Driver issues (install CP210x or CH340 drivers)"
    exit 1
fi

echo ""

# Step 4: Smoke test instructions
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Step 4: CLI Smoke Test"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo ""
echo "Opening serial monitor at 115200 baud..."
echo ""
echo -e "${YELLOW}Expected output on boot:${NC}"
cat << 'EOF'
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
EOF

echo ""
echo -e "${YELLOW}Run these smoke test commands:${NC}"
echo ""
echo "1. help"
echo "   Expected: List of all commands"
echo ""
echo "2. sys info"
echo "   Expected: Board name, chip, flash size, heap info"
echo ""
echo "3. hal caps"
echo "   Expected: GPIO/ADC/PWM/I2C/SPI/UART counts"
echo ""
echo "4. ep list"
echo "   Expected: List of registered endpoints"
echo ""
echo "5. bind gpio.dout gpio.dout.2"
echo "   Expected: device_id=1"
echo ""
echo "6. dev list"
echo "   Expected: dev1: gpio.dout @ gpio.dout.2 [READY]"
echo ""
echo "7. param set 1 state 1"
echo "   Expected: OK (LED/pin should change if connected)"
echo ""
echo "8. param get 1 state"
echo "   Expected: state=1"
echo ""
echo "9. schema 1"
echo "   Expected: Device schema (settings, signals, commands)"
echo ""
echo "10. log tail"
echo "    Expected: Recent log entries"
echo ""
echo "Press Ctrl+C to exit monitor when done."
echo ""
echo "Starting monitor in 3 seconds..."
sleep 3

pio device monitor

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${GREEN}Smoke test complete!${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "If all commands worked, PocketOS v1.0 is functioning correctly."
echo ""
