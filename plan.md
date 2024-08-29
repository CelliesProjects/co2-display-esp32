# CO<sup>2</sup> display esp32

### Where is this going?

A display showing the current temperature, relative humidity and CO<sup>2</sup> level as ppm.<br>
Save the average values -once a minute?- to a SD card
Also showing a bargraph for Temp, Humidity and CO<sup>2</sup> history.<br>
Show a ntp synced clock based on the users location/timezone.

### Program flow and setup

There will be separate tasks for

- Temp, humidity and CO<sup>2</sup> sensors.
- Display and touchpanel
- SD card access
- Webserver/websocket

The tasks will be an infinite loop FSM after the initial HW setup. 

### Hardware setup

- The sensors can be read sequentially from the same task. No need for mutex. They need to be on a second I2C bus as the internal bus is not exposed and used by the touchpanel.
- The touchpanel is on the internal I2C and can be controlled by the same task as the display?
- The display and SD card share the SPI bus and need to share it somehow.

### Locks and mutexes

The SPI bus is shared between the display task and the sd card task. This is done by a binary mutex.

### The shitty thing

Although it is powerewd by a esp32-s3 it seems the usb protocols are not implemented and logging is only possible over UART on GPIOs 43 and 44. Still have to look into that.

It means for now that we can log over serial until the CO<sup>2</sup> sensors are started as they connect through GPIO 43 and 44. Once the display gets started we need to log to the display. This means the display task is the first one to start so the other tasks can send messages to it when they start running. And it needs some log type thingies to start with.

