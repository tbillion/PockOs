# PocketOS

PocketOS is an embedded operating system framework for ESP32 development using PlatformIO and the Arduino framework.

## Features

- **Core Components**:
  - HAL (Hardware Abstraction Layer)
  - Resources Management
  - Endpoints
  - Devices
  - Intents
  - Schema Validation
  - Logger
  - Persistence

- **Drivers**:
  - GPIO Digital Output
  - GPIO Digital Input
  - PWM Output (conditional compilation with `POCKETOS_ENABLE_PWM`)
  - ADC Input (conditional compilation with `POCKETOS_ENABLE_ADC`)
  - I2C Bus (conditional compilation with `POCKETOS_ENABLE_I2C`)

- **CLI**: Interactive command-line interface via serial monitor

## Project Structure

```
PocketOS/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp           # Entry point
│   └── pocketos/
│       ├── core/          # Core system components
│       │   ├── hal.h/cpp
│       │   ├── resources.h/cpp
│       │   ├── endpoints.h/cpp
│       │   ├── devices.h/cpp
│       │   ├── intents.h/cpp
│       │   ├── schema.h/cpp
│       │   ├── logger.h/cpp
│       │   └── persistence.h/cpp
│       ├── drivers/       # Hardware drivers
│       │   ├── gpio_dout.h/cpp
│       │   ├── gpio_din.h/cpp
│       │   ├── pwm_out.h/cpp
│       │   ├── adc_in.h/cpp
│       │   └── i2c_bus.h/cpp
│       └── cli/           # Command-line interface
│           ├── cli.h
│           └── cli.cpp
└── README.md
```

## Requirements

- [PlatformIO](https://platformio.org/) installed
- ESP32 development board
- USB cable for programming and serial communication

## Getting Started

### Build the Project

To compile the project:

```bash
pio run
```

or

```bash
platformio run
```

### Upload to ESP32

To upload the firmware to your ESP32 board:

```bash
pio run --target upload
```

or

```bash
platformio run --target upload
```

### Monitor Serial Output

To open the serial monitor and interact with the CLI:

```bash
pio device monitor
```

or

```bash
platformio device monitor
```

To exit the monitor, press `Ctrl+C`.

### Build, Upload, and Monitor (One Command)

You can combine all three operations:

```bash
pio run --target upload && pio device monitor
```

## Configuration

The project is configured in `platformio.ini` with the following settings:

- **Platform**: ESP32 (espressif32)
- **Board**: ESP32 Dev Module (esp32dev)
- **Framework**: Arduino
- **Monitor Speed**: 115200 baud
- **Build Flags**:
  - `POCKETOS_ENABLE_I2C` - Enable I2C bus driver
  - `POCKETOS_ENABLE_ADC` - Enable ADC input driver
  - `POCKETOS_ENABLE_PWM` - Enable PWM output driver

## CLI Commands

Once uploaded and monitoring, you can use the following commands:

- `help` - Display available commands
- `status` - Show system status
- `version` - Show PocketOS version

## Development

The PocketOS framework uses namespaces to organize code:

- `PocketOS::` - Core framework components
- `PocketOS::Drivers::` - Hardware driver components

All components are implemented as static classes for easy access throughout the application.

## License

See LICENSE file for details.
