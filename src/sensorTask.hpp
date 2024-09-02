#ifndef _SENSOR_TASK_
#define _SENSOR_TASK_

#include <Arduino.h>
#include <driver/uart.h>
#include <Adafruit_SHT31.h>
#include <s8_uart.h>

#include "displayMessageStruct.hpp"
extern QueueHandle_t displayQueue;

#define PIN_SENSOR_SCL 40
#define PIN_SENSOR_SDA 2

//  because Wire is only internally connected to the touchpad
//  the sht31 sensor has to be on Wire1 and routed
//  through the exposed GPIOs on the back of the device
static TwoWire I2Csensor = TwoWire(1);
static Adafruit_SHT31 sht31(&I2Csensor);

// co2 sensor is a serial device 9600-8-n-1
#define PIN_S8_RXD 43
#define PIN_S8_TXD 44

static HardwareSerial S8_serial(2);
static S8_UART *sensor_S8;

void sensorTask(void *parameter);
#endif