# co2 display esp32

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/2681ce16ae05470f9b205de81a437ed3)](https://app.codacy.com/gh/CelliesProjects/co2-display-esp32?utm_source=github.com&utm_medium=referral&utm_content=CelliesProjects/co2-display-esp32&utm_campaign=Badge_Grade)

![SAM_3242](https://github.com/user-attachments/assets/76c282f7-baca-4862-9028-22992eba1542)

This project is designed to measure and display CO2 levels using an ESP32 microcontroller.<br>
It reads data from a Senseair S8 CO2 sensor and an SHT31 temperature and humidity sensor, displaying the real-time readings.

The sensors are on separate sensor board and are read over websocket.<br>
This setup is chosen because the CO2 sensor uses over 300mA when reading and this made the tft screen flicker during the reading.

The sensor code is still private atm. but will be made public soon.

## Features

- CO2 monitoring with the Senseair S8 sensor.
- Temperature and humidity monitoring with the SHT31 sensor.
- Built using the ESP32 microcontroller.
- Configured with PlatformIO for easy development.

### Installation

1. Clone this repository.<br>
```bash
git clone https://github.com/CelliesProjects/co2-display-esp32.git
```

 2. Add a secrets.h file to the include folder with the following content:<br>
 ```c++
#ifndef SECRETS
#define SECRETS

const char *WIFI_SSID = "network";
const char *WIFI_PSK = "password";

const char *VISUAL_CROSSING_CITY = "city";
const char *VISUAL_CROSSING_COUNTRY = "country";
const char *VISUAL_CROSSING_API_KEY = "your api key";

#endif
```
3. Build and upload the firmware to the ESP32 board using PlatformIO.

## License
This project is licensed under the MIT License.

