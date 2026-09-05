# Bare-metal STM32F411 + MPU-6050

Simple bare-metal (register-level) project for **STM32F411** that reads the **MPU-6050** accelerometer/gyroscope over I2C.

## Current status
**In progress**

- Clock (HSE 25 MHz)
- UART2 debug output (115200)
- I2C1 driver (PB6/PB7) with bus recovery
- MPU-6050 init + burst read (accel + gyro + temperature)

## Planned
- Add a control loop using the sensor data
- Closed-loop control (example: balance, orientation, or simple feedback)

## Hardware
- STM32F411 (Black Pill / Nucleo style)
- MPU-6050
  - VCC → 3.3 V
  - GND → GND
  - SCL → PB6
  - SDA → PB7
  - AD0 → GND (address 0x68)
  - External 2.2–4.7 kΩ pull-ups on SCL & SDA

## Build
Open in STM32CubeIDE (or any arm-none-eabi toolchain) and flash.
