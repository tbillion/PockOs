#ifndef POCKETOS_HAL_H
#define POCKETOS_HAL_H

#include <Arduino.h>

namespace PocketOS {

class HAL {
public:
    static void init();
    static void update();
    
    // Time functions
    static unsigned long getMillis();
    static void delay(unsigned long ms);
    
    // Board information
    static const char* getBoardName();
    static const char* getChipFamily();
    static uint32_t getFlashSize();
    static uint32_t getHeapSize();
    static uint32_t getFreeHeap();
    
    // Capability counts
    static int getGPIOCount();
    static int getADCCount();
    static int getPWMCount();
    static int getI2CCount();
    static int getSPICount();
    static int getUARTCount();
    
    // Safe pins (platform-specific)
    static bool isPinSafe(int pin);
    static void getSafePins(int* pins, int* count);
    
    // GPIO functions
    static void gpioMode(int pin, int mode);
    static void gpioWrite(int pin, bool value);
    static bool gpioRead(int pin);
    
    // ADC functions
    static int adcRead(int channel);
    static float adcReadVoltage(int channel, float vref = 3.3);
    
    // PWM functions
    static bool pwmInit(int pin, int channel, int frequency = 5000);
    static void pwmWrite(int channel, int dutyCycle);
    static void pwmWritePercent(int channel, float percent);
    
    // I2C functions
    static bool i2cInit(int busNum, int sda = -1, int scl = -1);
    static bool i2cProbe(int busNum, uint8_t address);
    static bool i2cWrite(int busNum, uint8_t address, uint8_t* data, size_t len);
    static bool i2cRead(int busNum, uint8_t address, uint8_t* data, size_t len);
    
private:
    static bool initialized;
};

} // namespace PocketOS

#endif // POCKETOS_HAL_H
