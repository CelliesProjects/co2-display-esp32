# CO<sub>2</sub> display esp32

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/bdbc66cdcdfd46ab99144949714cc822)](https://app.codacy.com/gh/CelliesProjects/co2-display-esp32/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

![SAM_3242](https://github.com/user-attachments/assets/76c282f7-baca-4862-9028-22992eba1542)

This is a [PlatformIO](https://platformio.org/) project designed to display CO<sub>2</sub> levels using a [Panel ESP32-4848S040](https://homeding.github.io/boards/esp32s3/panel-4848S040.htm).<br>
It reads data from a Senseair S8 CO<sub>2</sub> sensor and an SHT31 temperature and humidity sensor, displaying the real-time readings and the history of the last 2 hours.

Sensors are on separate sensor board and are retrieved over websocket.<br>
This setup is chosen because the SenseAir S8 CO<sub>2</sub> sensor uses up to 300mA when reading and this makes the tft screen flicker during reading.<br>
You can find the sensor code at the [SensorHub repo](https://github.com/CelliesProjects/SensorHub).

There is also weather forecast data shown which is retrieved from [visualcrossing.com](https://www.visualcrossing.com/).<br>
You will need to setup an account with visualcrossing to be able to download weather data but registration and 1000 free API credits per day are included in the free accounts.<br>
You can setup your free account [here](https://www.visualcrossing.com/weather-api).

### Installation

Assuming you already have the [SensorHub](https://github.com/CelliesProjects/SensorHub) running and a [visualcrossing account](https://www.visualcrossing.com/weather-api) API key you can use these steps to install:

1.  Clone this repository.
```bash
git clone https://github.com/CelliesProjects/co2-display-esp32.git
```

 2.  Open the extracted folder in PlatformIO and add a `secrets.h` file to the `include` folder with the following content:<br>
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
3. Build and upload the firmware to the ESP32.

### Done!

If all went well the board will now search for the sensorhub and connect automagically.

## License
This project is licensed under the MIT License.

