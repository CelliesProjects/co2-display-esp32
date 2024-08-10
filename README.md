 # BOARD: ESP32-4848S040

| **I2C**|  |
|-------|---|
| SDA| 19 |
| SCL | 45 |

| **TOUCH PANEL I2C ID** |  
|--------------| 
| 0x5D     |


| **SPI** | |
|-------|---|
| SCL | 48 |
| MOSI | 47 |
| MISO | 41 |

|-------|---|
|-------|---|
| BACK_LIGHT | 38 |
| CS CDCARD | 42 |

The GPIO that control the relays are the only free GPIO on this panel.
|RELAY|PIN|
|-------|---|
|1| 40
|2| 2
|3| 1

More info:

https://homeding.github.io/boards/esp32s3/panel-4848S040.htm

https://github.com/arendst/Tasmota/discussions/20527

https://michiel.vanderwulp.be/domotica/Modules/SmartDisplay-ESP32-S3-4.0inch/software.html

https://github.com/moononournation/Arduino_GFX/issues/465 <-------WORKING!