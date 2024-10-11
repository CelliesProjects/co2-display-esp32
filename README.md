# co2 display esp32

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/bdbc66cdcdfd46ab99144949714cc822)](https://app.codacy.com/gh/CelliesProjects/co2-display-esp32/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

![SAM_3242](https://github.com/user-attachments/assets/76c282f7-baca-4862-9028-22992eba1542)

This project is designed to display CO2 levels using a [Panel ESP32-4848S040](https://homeding.github.io/boards/esp32s3/panel-4848S040.htm).<br>
It reads data from a Senseair S8 CO2 sensor and an SHT31 temperature and humidity sensor, displaying the real-time readings.

Sensors are on separate sensor board and are read over websocket.<br>
This setup is chosen because the CO2 sensor uses over 300mA when reading and this made the tft screen flicker during the reading.<br>
The sensor code is still private atm. but will be made public soon.

There is also weather forecast data shown which is retrieved from [visualcrossing.com](https://www.visualcrossing.com/).<br>
You will need to setup an account with visualcrossing to be able to download weather data but registration and 1000 free API credits per day are included in the free accounts.<br>
You can setup your free account [here](https://www.visualcrossing.com/weather-api).

## Features

-  CO2 monitoring with the Senseair S8 sensor.
-  Temperature and humidity monitoring with the SHT31 sensor.
-  Configured with PlatformIO for easy development.

### Installation

1.  Clone this repository.
```bash
git clone https://github.com/CelliesProjects/co2-display-esp32.git
```

 2.  Add a secrets.h file to the include folder with the following content:<br>
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

