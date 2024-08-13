 # BOARD: ESP32-4848S040

https://github.com/CelliesProjects/co2-display-esp32 

## I2C pins

| I2C| PIN |
|:--:|:--:|
|SDA|19|  
|SCL|45|  

Touchpanel is a I2C device

|TOUCH|ID|
|:--:|:--:|
|ID |0x5D|  

## SPI pins

| SPI |PIN |
|:--:|:--:|
|SCL|48|
|MOSI|47|
|MISO|41|

The sdcard is an SPI device.
|SDCARD|PIN|
|:--:|:--:|
|CS|42|

The display is connected through SPI -no MISO pin- and a lot of pins for the actual pixel data.
|DISPLAY|PIN|
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

The GPIO that control the relays are on the backside of the panel (with the relays removed)
|RELAY|PIN|
|:--:|:--:|
|1| 40 |
|2| 2 |
|3| 1 |

Also exposed on the backside are pins marked TXD and RXD.
|MARK| PIN|
|:--:|:--:|
| TX  | 43  |
| RX  | 44  |

A NS4168 dac is present on the board, but is not connected.<br>
Three zero ohm resistors are missing on the board.<br>
These resistors -if installed- connect the esp32 to the dac.<br>
Check to see if it is safe to solder these connections closed.

There is a pin conflict: `GPIO4` is used by the tft display as `B0` and also as `SDATA` for the NS4168<br>

| NS4168 | PIN |
|:--:|:--:|
|CTRL | ? |
|LRCLK | 36 |
|BCLK | 23 |
|SDATA | 4 |

More info:

https://homeding.github.io/boards/esp32s3/panel-4848S040.htm

https://github.com/arendst/Tasmota/discussions/20527

https://michiel.vanderwulp.be/domotica/Modules/SmartDisplay-ESP32-S3-4.0inch/software.html

https://github.com/moononournation/Arduino_GFX/issues/465 <-------WORKING!

https://lastminuteengineers.com/sht31-temperature-humidity-sensor-arduino-tutorial/