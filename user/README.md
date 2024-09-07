# Custom firmware for STC microcontroller of Zigbee CO2 sensor 

### Hardware: 
- `ZT3L` Tuya Zigbee module
- `STC8G1K17` STCmicro 8-bit 8051 microcontroller
- `MTP40-F`-like CO2 Sensor

```
Zigbee  <-UART->  STC  <-UART->  CO2 Sensor
                   ^
                   |
               Push Button
```

STC8G can be flashed using any USB-UART converter. It should be configured for 3.3v I/O since MCU is powered by 3.3v LDO. 

[UART connection scheme and other photos](https://github.com/rokiden/Tuya_CO2_STC8/issues/1)

### Software 

- SDCC (installed from DNF in Fedora)
- stcgal (installed from pip)
- debug.py contains UART parsers used for development

#### Commands:

- Build and flash `make && stcgal -P stc8g -p /dev/ttyUSB0 build/app.hex`
- Clean build artifacts `make clean`
- Format user/* sources `make format`

### Firmware behavior:

- For pairing hold the button for 2.5s
- LED indicates states:
  - *not paired*: regular blink with 1s pause
  - *paired*: blink every 10min when data sent to Zigbee network
  - *error*: regular N blinks with 1s pause, N = error code (defined in led.h)
- data acquisition period and filtering parameters are defined in sensor.h (by default: measure every 4.6875s, average 128 measurements, as a result, average value sent every 10 minutes)
- *error* is not a recoverable condition, power cycle to reset

### Links

Aliexpress Link: [NDIR CO2 Sensor Zigbee](https://sl.aliexpress.ru/p?key=bX53rm8) Firmware tested on white squared Zigbee version.

Docs:
- [STC8G family of Microcontrollers Reference Manual](http://www.stcmicro.com/datasheet/STC8G-en.pdf)
- [ZT3L Module Datasheet](https://developer.tuya.com/en/docs/iot/zt3l-module-datasheet?id=Ka438n1j8nuvu)
- [ZT3L UART Protocol](https://developer.tuya.com/en/docs/iot/tuya-zigbee-module-uart-communication-protocol?id=K9ear5khsqoty)
- [MTP40-F](https://www.tinytronics.nl/index.php?route=product/product/get_file&file=4660/MTP40-F-CO2-sensor-module-single-channel.pdf)
