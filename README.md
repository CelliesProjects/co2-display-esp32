 # BOARD: ESP32-4848S040

https://github.com/CelliesProjects/co2-display-esp32 

## Project status
Running. Display, touch and CO<sup>2</sup> working.

## I2C GPIOs

|  | GPIO |
|:--:|:--:|
|SDA|19|  
|SCL|45|  

Touchpanel is a gt911 I2C device

|GT911|ID|
|:--:|:--:|
|ID |0x5D|  

## SPI GPIOs

|   |GPIO |
|:--:|:--:|
|SCL|48|
|MOSI|47|
|MISO|41|

## The sdcard is SPI.

| |GPIO|
|:--:|:--:|
|CS|42|

## The display is connected through SPI -no MISO GPIO- and a lot of GPIOs for the actual pixel data.

|st7701s|GPIO|
|:--:|:--:|
|BACKLIGHT|38|
|CS|39|
|SCL|48|
|MOSI|47|
|DE|18|
|VSYNC|17|
|HSYNC|16|
|PCLK|21|
|R0|11|
|R1|12|
|R2|13|
|R3|14|
|R4|0|
|G0|8|
|G1|20|
|G2|3|
|G3|46|
|G4|9|
|G5|10|
|B0|4|
|B1|5|
|B2|6|
|B3|7|
|B4|15|
|

## The GPIO that control the relays are on the backside of the panel (with the relays removed)

|RELAY|GPIO|
|:--:|:--:|
|1| 40 |
|2| 2 |
|3| 1 |

## Exposed on the backside are GPIOs marked TXD and RXD.

|MARK| GPIO|
|:--:|:--:|
|TXD|43|
|RXD|44|


A NS4168 dac is present on the board, but is not connected because of three zero ohm bridges missing on the board.<br>
These bridges connect the `LRCLK`, `BCLK` and `SDATA` to the GPIO<br>
But there is a GPIO conflict here: `GPIO4` is used by the tft display as `B0` -pixeldata- and also as `SDATA` -sample data- by the NS4168<br>
I dont know if there is a way to share the GPIOs between the tft and dac

| NS4168 | GPIO |
|:--:|:--:|
|CTRL | NC? |
|LRCLK | 36 |
|BCLK | 23 |
|SDATA | 4 |

More info:


https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/protocols/esp_websocket_client.html

https://github.com/espressif/esp-idf/blob/v4.2/examples/protocols/websocket/main/websocket_example.c

https://www.visualcrossing.com/weather/weather-data-services/

https://github.com/visualcrossing/WeatherApi/blob/master/Arduino_samples_esp32/src/sketch.ino

https://github.com/esp-arduino-libs/ESP32_Display_Panel/blob/master/docs/Board_Instructions.md#shenzhen-jingcai-intelligent

https://homeding.github.io/boards/esp32s3/panel-4848S040.htm

https://github.com/arendst/Tasmota/discussions/20527

https://michiel.vanderwulp.be/domotica/Modules/SmartDisplay-ESP32-S3-4.0inch/software.html

https://github.com/moononournation/Arduino_GFX/issues/465 <-------WORKING!

https://lastminuteengineers.com/sht31-temperature-humidity-sensor-arduino-tutorial/

https://thingpulse.com/usb-settings-for-logging-with-the-esp32-s3-in-platformio/

https://docs.espressif.com/projects/arduino-esp32/en/latest/troubleshooting.html

https://www.makerfabs.com/esp32-s3-parallel-tft-with-touch-4-inch.html

## Example using Lovyan GFX
https://github.com/Makerfabs/ESP32-S3-Parallel-TFT-with-Touch-4inch/blob/main/example/lovyanGFX_demo

## About the capacitors on the sensor board
https://picaxeforum.co.uk/threads/5v-voltage-regulator-capacitor-values.30154/



   these are the headers for websocket messages

   a good message has the header on one line followed bij '\n'

   example: G:\n

   -  A: new saved average (C: H: T: ) to add to the history
   -  C: current co2 level
   -  G: history            used by both for client (request) and server
   -  H: current humidity
   -  P: ping used to keep the server informed that the client has not yet buggered off
   -  T: current temperature

timeout is 4 seconds no messages received before client reconnects
