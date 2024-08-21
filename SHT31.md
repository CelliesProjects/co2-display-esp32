
## Description

Sensiron Temperature/Humidity sensors are some of the finest & highest-accuracy devices you can get. And, finally we have some that have a true I2C interface for easy reading. The SHT31-D sensor has an excellent ±2% relative humidity and ±0.3°C accuracy for most uses.

Unlike earlier SHT sensors, this sensor has a true I2C interface, and (bonus!) even with two address options. It also is 3V or 5V compliant, so you can power and communicate with it using just about any microcontroller or microcomputer.

Such a lovely chip - so we spun up a breakout board with the SHT31-D and some supporting circuitry such as pullup resistors and capacitors. Each order comes with one fully assembled and tested PCB breakout and a small piece of header. You'll need to solder the header onto the PCB but it's fairly easy and takes only a few minutes even for a beginner.

## Power Pins:
-  `Vin` - this is the power pin. The chip can use 2.5-5VDC for power. To power the board, give it the same power as the logic level of your microcontroller - e.g. for a 5V micro like Arduino, use 5V. For a 3.3V controller like a Raspbery Pi, connect to 3.3V
-  `GND` - common ground for power and logic
 
## I2C Logic pins:
-  `SCL` - I2C clock pin, connect to your microcontrollers I2C clock line. This pin has a 10K pullup resistor to Vin
-  `SDA` - I2C data pin, connect to your microcontrollers I2C data line. This pin has a 10K pullup resistor to Vin
 
## Other Pins:
-  `ADR` - This is the I2C address selection pin. This pin has a 10K pull down resistor to make the default I2C address 0x44. You can tie this pin to Vin to make the address 0x45
-  `RST` - Hardware reset pint. Has a 10K pullup on it to make the chip active by default. Connect to ground to do a hardware reset!
-  `ALR` - Alert/Interrupt output. You can set up the sensor to alert you when an event has occured. Check the datasheet for how you can set up the alerts