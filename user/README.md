# Custom firmware for STC microcontroller of Zigbee CO2 sensor 

The main goal of this project is to fix the flood bug in the original firmware. The custom firmware reports the average CO2 concentration over a 10-minute period. The average value is calculated based on 128 measurements.

### Quick Guide
- Download lastest [Release](https://github.com/rokiden/Tuya_CO2_STC8/releases)
- Connect USB-UART [Scheme](https://github.com/rokiden/Tuya_CO2_STC8/issues/1)
- `pip install stcgal`
- `stcgal -P stc8g -p /dev/ttyUSB0 app.hex` (replace ttyUSB0 for your case)

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

- stcgal programmer (installed from pip)
- SDCC compiler (installed from DNF in Fedora)
- debug.py contains UART parsers used for development

#### Commands:

- Flash [pre-compiled](https://github.com/rokiden/Tuya_CO2_STC8/releases) hex `stcgal -P stc8g -p /dev/ttyUSB0 app.hex`
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

### Zigbee2MQTT

Original device identified as `TS0601 _TZE204_ogkdpgy2` by Zigbee2MQTT and not supported. [Fix](https://github.com/Koenkk/zigbee2mqtt/issues/23205#issuecomment-2235297039)

Modified firmware uses the same identification code.

### Links

Aliexpress Link: [NDIR CO2 Sensor Zigbee](https://sl.aliexpress.ru/p?key=bX53rm8) Firmware tested on white squared Zigbee version.

Docs:
- [STC8G family of Microcontrollers Reference Manual](http://www.stcmicro.com/datasheet/STC8G-en.pdf)
- [ZT3L Module Datasheet](https://developer.tuya.com/en/docs/iot/zt3l-module-datasheet?id=Ka438n1j8nuvu)
- [ZT3L UART Protocol](https://developer.tuya.com/en/docs/iot/tuya-zigbee-module-uart-communication-protocol?id=K9ear5khsqoty)
- [MTP40-F](https://www.tinytronics.nl/index.php?route=product/product/get_file&file=4660/MTP40-F-CO2-sensor-module-single-channel.pdf)

## Custom case

Custom case with minimal size. FreeCAD project and stl for 3d printing located in 'user' dir.

[Photo](https://github.com/rokiden/Tuya_CO2_STC8/issues/1#issuecomment-2395081453)

