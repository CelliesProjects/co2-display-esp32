 # BOARD: ESP32-4848S040

| I2C| PIN |
|-------|---|  
| SDA| 19 |  
| SCL | 45 |  

| I2C | TOUCH|   
|--------------|--|  
|  ID | 0x5D   |  


| **SPI** |PIN |
|-------|---|
| SCL | 48 |
| MOSI | 47 |
| MISO | 41 |

|-------|---|
|-------|---|
| BACKLIGHT | 38 |
| CS CDCARD | 42 |

The GPIO that control the relays are on the backside of the panel (with the relays removed)
|RELAY|PIN|
|-------|---|
|1| 40 |
|2| 2 |
|3| 1 |

Also exposed on the backside are pins marked TXD and RXD
|MARK| PIN|
|-----|-----|
| TX  | 43  |
| RX  | 44  |

More info:

https://homeding.github.io/boards/esp32s3/panel-4848S040.htm

https://github.com/arendst/Tasmota/discussions/20527

https://michiel.vanderwulp.be/domotica/Modules/SmartDisplay-ESP32-S3-4.0inch/software.html

https://github.com/moononournation/Arduino_GFX/issues/465 <-------WORKING!

https://lastminuteengineers.com/sht31-temperature-humidity-sensor-arduino-tutorial/