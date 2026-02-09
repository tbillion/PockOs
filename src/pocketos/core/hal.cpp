#include "hal.h"
#include "logger.h"

#ifdef ESP32
#include <Wire.h>
#endif

namespace PocketOS {

bool HAL::initialized = false;

void HAL::init() {
    if (!initialized) {
        #ifdef ESP32
        // ESP32 specific initialization
        #endif
        Logger::info("HAL initialized");
        initialized = true;
    }
}

void HAL::update() {
    // Update hardware abstraction layer
}

unsigned long HAL::getMillis() {
    return millis();
}

void HAL::delay(unsigned long ms) {
    ::delay(ms);
}

const char* HAL::getBoardName() {
    #ifdef ESP32
    return "ESP32";
    #elif defined(ESP8266)
    return "ESP8266";
    #elif defined(ARDUINO_ARCH_RP2040)
    return "RP2040";
    #else
    return "Unknown";
    #endif
}

const char* HAL::getChipFamily() {
    #ifdef ESP32
    return "ESP32";
    #elif defined(ESP8266)
    return "ESP8266";
    #elif defined(ARDUINO_ARCH_RP2040)
    return "RP2040";
    #else
    return "Unknown";
    #endif
}

uint32_t HAL::getFlashSize() {
    #ifdef ESP32
    return ESP.getFlashChipSize();
    #elif defined(ESP8266)
    return ESP.getFlashChipSize();
    #else
    return 0;
    #endif
}

uint32_t HAL::getHeapSize() {
    #ifdef ESP32
    return ESP.getHeapSize();
    #elif defined(ESP8266)
    return 81920; // Typical ESP8266 heap
    #else
    return 0;
    #endif
}

uint32_t HAL::getFreeHeap() {
    #ifdef ESP32
    return ESP.getFreeHeap();
    #elif defined(ESP8266)
    return ESP.getFreeHeap();
    #else
    return 0;
    #endif
}

int HAL::getGPIOCount() {
    #ifdef ESP32
    return 40; // ESP32 has up to GPIO 39
    #elif defined(ESP8266)
    return 17; // ESP8266 has GPIO 0-16
    #elif defined(ARDUINO_ARCH_RP2040)
    return 30; // RP2040 has GPIO 0-29
    #else
    return 0;
    #endif
}

int HAL::getADCCount() {
    #ifdef POCKETOS_ENABLE_ADC
    #ifdef ESP32
    return 18; // ESP32 has 18 ADC channels
    #elif defined(ESP8266)
    return 1; // ESP8266 has 1 ADC
    #elif defined(ARDUINO_ARCH_RP2040)
    return 4; // RP2040 has 4 ADC channels
    #endif
    #endif
    return 0;
}

int HAL::getPWMCount() {
    #ifdef POCKETOS_ENABLE_PWM
    #ifdef ESP32
    return 16; // ESP32 has 16 PWM channels
    #elif defined(ESP8266)
    return 8; // ESP8266 can do PWM on most pins
    #elif defined(ARDUINO_ARCH_RP2040)
    return 16; // RP2040 has 16 PWM channels
    #endif
    #endif
    return 0;
}

int HAL::getI2CCount() {
    #ifdef POCKETOS_ENABLE_I2C
    #ifdef ESP32
    return 2; // ESP32 has 2 I2C buses
    #elif defined(ESP8266)
    return 1; // ESP8266 has 1 I2C
    #elif defined(ARDUINO_ARCH_RP2040)
    return 2; // RP2040 has 2 I2C
    #endif
    #endif
    return 0;
}

int HAL::getSPICount() {
    #ifdef ESP32
    return 3; // ESP32 has 3 SPI
    #elif defined(ESP8266)
    return 1; // ESP8266 has 1 SPI
    #elif defined(ARDUINO_ARCH_RP2040)
    return 2; // RP2040 has 2 SPI
    #else
    return 1;
    #endif
}

int HAL::getUARTCount() {
    #ifdef ESP32
    return 3; // ESP32 has 3 UART
    #elif defined(ESP8266)
    return 2; // ESP8266 has 2 UART
    #elif defined(ARDUINO_ARCH_RP2040)
    return 2; // RP2040 has 2 UART
    #else
    return 1;
    #endif
}

bool HAL::isPinSafe(int pin) {
    #ifdef ESP32
    // Avoid strapping pins and flash pins
    if (pin == 0 || pin == 2 || pin == 5 || pin == 12 || pin == 15) return false;
    if (pin >= 6 && pin <= 11) return false; // Flash pins
    if (pin >= 34 && pin <= 39) return true; // Input only, but safe
    if (pin < 40) return true;
    #elif defined(ESP8266)
    // Avoid flash pins
    if (pin >= 6 && pin <= 11) return false;
    if (pin <= 16) return true;
    #elif defined(ARDUINO_ARCH_RP2040)
    if (pin < 30) return true;
    #endif
    return false;
}

void HAL::getSafePins(int* pins, int* count) {
    *count = 0;
    for (int i = 0; i < getGPIOCount(); i++) {
        if (isPinSafe(i)) {
            pins[*count] = i;
            (*count)++;
        }
    }
}

void HAL::gpioMode(int pin, int mode) {
    pinMode(pin, mode);
}

void HAL::gpioWrite(int pin, bool value) {
    digitalWrite(pin, value ? HIGH : LOW);
}

bool HAL::gpioRead(int pin) {
    return digitalRead(pin) == HIGH;
}

int HAL::adcRead(int channel) {
    #ifdef POCKETOS_ENABLE_ADC
    return analogRead(channel);
    #else
    return 0;
    #endif
}

float HAL::adcReadVoltage(int channel, float vref) {
    #ifdef POCKETOS_ENABLE_ADC
    int raw = analogRead(channel);
    #ifdef ESP32
    return (raw * vref) / 4095.0;
    #elif defined(ESP8266)
    return (raw * vref) / 1023.0;
    #else
    return (raw * vref) / 1023.0;
    #endif
    #else
    return 0.0;
    #endif
}

bool HAL::pwmInit(int pin, int channel, int frequency) {
    #ifdef POCKETOS_ENABLE_PWM
    #ifdef ESP32
    ledcSetup(channel, frequency, 8);
    ledcAttachPin(pin, channel);
    return true;
    #else
    // For ESP8266/RP2040, analogWrite handles PWM
    pinMode(pin, OUTPUT);
    return true;
    #endif
    #else
    return false;
    #endif
}

void HAL::pwmWrite(int channel, int dutyCycle) {
    #ifdef POCKETOS_ENABLE_PWM
    #ifdef ESP32
    ledcWrite(channel, dutyCycle);
    #endif
    #endif
}

void HAL::pwmWritePercent(int channel, float percent) {
    #ifdef POCKETOS_ENABLE_PWM
    #ifdef ESP32
    int duty = (int)(255.0 * percent / 100.0);
    ledcWrite(channel, duty);
    #endif
    #endif
}

bool HAL::i2cInit(int busNum, int sda, int scl, uint32_t speedHz) {
    #ifdef POCKETOS_ENABLE_I2C
    #ifdef ESP32
    if (busNum == 0) {
        if (sda < 0) sda = 21;
        if (scl < 0) scl = 22;
        Wire.begin(sda, scl, speedHz);
        Logger::info("I2C initialized: SDA=" + String(sda) + ", SCL=" + String(scl) + ", Speed=" + String(speedHz) + "Hz");
        return true;
    }
    #elif defined(ESP8266)
    if (sda < 0) sda = 4;
    if (scl < 0) scl = 5;
    Wire.begin(sda, scl);
    Wire.setClock(speedHz);
    Logger::info("I2C initialized: SDA=" + String(sda) + ", SCL=" + String(scl) + ", Speed=" + String(speedHz) + "Hz");
    return true;
    #endif
    #endif
    return false;
}

bool HAL::i2cProbe(int busNum, uint8_t address) {
    #ifdef POCKETOS_ENABLE_I2C
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
    #else
    return false;
    #endif
}

bool HAL::i2cWrite(int busNum, uint8_t address, uint8_t* data, size_t len) {
    #ifdef POCKETOS_ENABLE_I2C
    Wire.beginTransmission(address);
    Wire.write(data, len);
    return Wire.endTransmission() == 0;
    #else
    return false;
    #endif
}

bool HAL::i2cRead(int busNum, uint8_t address, uint8_t* data, size_t len) {
    #ifdef POCKETOS_ENABLE_I2C
    Wire.requestFrom(address, len);
    size_t i = 0;
    while (Wire.available() && i < len) {
        data[i++] = Wire.read();
    }
    return i == len;
    #else
    return false;
    #endif
}

bool HAL::i2cScan(int busNum, uint8_t* addresses, int* count, int maxCount) {
    #ifdef POCKETOS_ENABLE_I2C
    *count = 0;
    for (uint8_t addr = 1; addr < 127 && *count < maxCount; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            addresses[(*count)++] = addr;
        }
    }
    return true;
    #else
    return false;
    #endif
}

} // namespace PocketOS
