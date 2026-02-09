#!/bin/bash
# Build verification script for environments with network restrictions
# This script verifies code structure and performs basic validation

set -e

echo "================================"
echo "PocketOS Build Verification"
echo "================================"
echo ""

PROJECT_DIR="/home/runner/work/PockOs/PockOs"
cd "$PROJECT_DIR"

# Color codes
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

pass_count=0
fail_count=0
warn_count=0

check_pass() {
    echo -e "${GREEN}✓${NC} $1"
    ((pass_count++))
}

check_fail() {
    echo -e "${RED}✗${NC} $1"
    ((fail_count++))
}

check_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
    ((warn_count++))
}

echo "1. Checking project structure..."
if [ -f "platformio.ini" ]; then
    check_pass "platformio.ini exists"
else
    check_fail "platformio.ini missing"
fi

if [ -f "src/main.cpp" ]; then
    check_pass "src/main.cpp exists"
else
    check_fail "src/main.cpp missing"
fi

if [ -d "src/pocketos/core" ]; then
    check_pass "src/pocketos/core/ exists"
else
    check_fail "src/pocketos/core/ missing"
fi

if [ -d "src/pocketos/drivers" ]; then
    check_pass "src/pocketos/drivers/ exists"
else
    check_fail "src/pocketos/drivers/ missing"
fi

if [ -d "src/pocketos/cli" ]; then
    check_pass "src/pocketos/cli/ exists"
else
    check_fail "src/pocketos/cli/ missing"
fi

echo ""
echo "2. Checking required core modules..."
required_core_modules=(
    "intent_api"
    "hal"
    "resource_manager"
    "endpoint_registry"
    "device_registry"
    "capability_schema"
    "logger"
    "persistence"
)

for module in "${required_core_modules[@]}"; do
    if [ -f "src/pocketos/core/${module}.h" ] && [ -f "src/pocketos/core/${module}.cpp" ]; then
        check_pass "Core module: $module"
    else
        check_fail "Core module missing: $module"
    fi
done

echo ""
echo "3. Checking driver modules..."
if [ -f "src/pocketos/drivers/gpio_dout_driver.h" ] && [ -f "src/pocketos/drivers/gpio_dout_driver.cpp" ]; then
    check_pass "GPIO digital output driver"
else
    check_fail "GPIO digital output driver missing"
fi

echo ""
echo "4. Checking CLI module..."
if [ -f "src/pocketos/cli/cli.h" ] && [ -f "src/pocketos/cli/cli.cpp" ]; then
    check_pass "CLI module"
else
    check_fail "CLI module missing"
fi

echo ""
echo "5. Checking file count..."
cpp_count=$(find src -name "*.cpp" | wc -l)
h_count=$(find src -name "*.h" | wc -l)
echo "   C++ files: $cpp_count"
echo "   Header files: $h_count"

if [ $cpp_count -ge 20 ] && [ $h_count -ge 20 ]; then
    check_pass "Sufficient source files ($cpp_count .cpp, $h_count .h)"
else
    check_warn "Few source files ($cpp_count .cpp, $h_count .h)"
fi

echo ""
echo "6. Checking for basic syntax errors..."
syntax_errors=0

# Create a minimal Arduino.h stub for syntax checking
mkdir -p /tmp/arduino_stub
cat > /tmp/arduino_stub/Arduino.h << 'EOF'
#ifndef Arduino_h
#define Arduino_h
#include <stdint.h>
#include <string.h>
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
typedef bool boolean;
class String {
public:
    String() {}
    String(const char*) {}
    String(int) {}
    int length() const { return 0; }
    void trim() {}
    String substring(int) const { return String(); }
    String substring(int, int) const { return String(); }
    bool startsWith(const String&) const { return false; }
    int indexOf(char) const { return -1; }
    int toInt() const { return 0; }
    const char* c_str() const { return ""; }
    String& operator=(const char*) { return *this; }
    bool operator==(const String&) const { return false; }
    bool operator==(const char*) const { return false; }
    String operator+(const String&) const { return String(); }
};
class Stream {};
class HardwareSerial : public Stream {
public:
    void begin(long) {}
    void println(const char*) {}
    void println() {}
    void print(const char*) {}
    void print(char) {}
    void print(int) {}
    void print(const String&) {}
    int available() { return 0; }
    int read() { return 0; }
};
extern HardwareSerial Serial;
void pinMode(int, int) {}
void digitalWrite(int, int) {}
int digitalRead(int) { return 0; }
int analogRead(int) { return 0; }
void analogReadResolution(int) {}
void delay(unsigned long) {}
unsigned long millis() { return 0; }
void ledcSetup(int, int, int) {}
void ledcAttachPin(int, int) {}
void ledcWrite(int, int) {}
class TwoWire {
public:
    void begin(int, int) {}
    void beginTransmission(uint8_t) {}
    size_t write(uint8_t*, size_t) { return 0; }
    size_t write(uint8_t) { return 0; }
    uint8_t endTransmission(bool = true) { return 0; }
    uint8_t requestFrom(uint8_t, uint8_t) { return 0; }
    size_t requestFrom(uint8_t, size_t) { return 0; }
    int available() { return 0; }
    int read() { return 0; }
};
extern TwoWire Wire;
class ESP32Class {
public:
    uint32_t getFlashChipSize() { return 0; }
    uint32_t getHeapSize() { return 0; }
    uint32_t getFreeHeap() { return 0; }
};
extern ESP32Class ESP;
class Preferences {
public:
    bool begin(const char*, bool) { return true; }
    size_t putString(const char*, String) { return 0; }
    String getString(const char*, const char* = "") { return String(); }
    bool remove(const char*) { return true; }
    void clear() {}
};
#endif
EOF

echo "   Performing syntax validation..."

for file in $(find src -name "*.cpp" -o -name "*.h"); do
    g++ -std=c++17 -fsyntax-only \
        -I/tmp/arduino_stub \
        -I./src \
        -I./src/pocketos/core \
        -I./src/pocketos/drivers \
        -I./src/pocketos/cli \
        -DARDUINO=10810 \
        -DESP32 \
        -DPOCKETOS_ENABLE_I2C \
        -DPOCKETOS_ENABLE_ADC \
        -DPOCKETOS_ENABLE_PWM \
        -Wno-unknown-pragmas \
        -Wno-register \
        "$file" 2>&1 | grep -q "error:" && {
            check_fail "Syntax error in $file"
            ((syntax_errors++))
        } || true
done

if [ $syntax_errors -eq 0 ]; then
    check_pass "No major syntax errors detected"
fi

echo ""
echo "7. Checking PlatformIO installation..."
if command -v pio &> /dev/null || python3 -m platformio --version &> /dev/null 2>&1; then
    check_pass "PlatformIO is installed"
    
    if [ -d "$HOME/.platformio/platforms/espressif32" ]; then
        check_pass "ESP32 platform is installed"
    else
        check_warn "ESP32 platform not installed (network issue)"
    fi
else
    check_warn "PlatformIO not installed"
fi

echo ""
echo "8. Checking documentation..."
if [ -f "README.md" ]; then
    check_pass "README.md exists"
    
    if grep -q "PocketOS v1" README.md; then
        check_pass "README contains v1 documentation"
    fi
fi

if [ -f "docs/BUILD_TROUBLESHOOTING.md" ]; then
    check_pass "Build troubleshooting guide exists"
fi

echo ""
echo "================================"
echo "Verification Summary"
echo "================================"
echo -e "${GREEN}Passed:${NC} $pass_count"
echo -e "${YELLOW}Warnings:${NC} $warn_count"
echo -e "${RED}Failed:${NC} $fail_count"
echo ""

if [ $fail_count -eq 0 ]; then
    echo -e "${GREEN}✓ Build verification PASSED${NC}"
    echo ""
    echo "Code structure is valid and ready for compilation."
    echo "Note: Full build requires ESP32 toolchain installation."
    echo "See docs/BUILD_TROUBLESHOOTING.md for details."
    exit 0
else
    echo -e "${RED}✗ Build verification FAILED${NC}"
    echo "Please fix the errors above."
    exit 1
fi
