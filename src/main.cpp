#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <FS.h>
#include <driver/uart.h>

#include <Adafruit_SHT31.h>
#include <s8_uart.h>

extern void displayTask(void* parameter);
TaskHandle_t displayTaskHandle = nullptr;

// sensor stuff
#define PIN_SENSOR_SCL 40
#define PIN_SENSOR_SDA 2

//  because Wire is only internally connected to the touchpad
//  the sht31 sensor has to be on Wire1 and routed
//  through the exposed GPIOs on the back of the device
TwoWire I2Csensor = TwoWire(1);

Adafruit_SHT31 sht31(&I2Csensor);

// co2 sensor is a serial device 9600-8-n-1
#define PIN_S8_RXD 43
#define PIN_S8_TXD 44

HardwareSerial S8_serial(2);
S8_UART *sensor_S8;

void setup()
{
    Serial.begin(115200);
    //Serial.setDebugOutput(true);

    // mount sd card
    SPI.begin(48, 41, 47);
    if (!SD.begin(42))
        Serial.println("SD card not found");
    else
        Serial.println("SD card mounted");

    // wait for a signal from the display task before continuing
    //  direct from task signal?
    //  or send a message to the display queue
    xTaskCreate(displayTask, NULL, 4096, NULL, 10, &displayTaskHandle);

    Serial.println("Setting Second I2C pins");

    // set second I2C interface pins
    I2Csensor.setPins(PIN_SENSOR_SDA, PIN_SENSOR_SCL);

    Serial.println("Starting sht31");

    bool result = sht31.begin(SHT31_DEFAULT_ADDR);
    Serial.printf("sht31 sensor is %s\n", result ? "found" : "absent");


    //Serial.end();


    delay(1000);
    Serial.println("Closing serial - logging continues on display");
    Serial.flush();

    // start SenseAir S8 sensor on TXD 43 and RXD 44
    S8_serial.setPins(PIN_S8_RXD, PIN_S8_TXD);
    S8_serial.begin(S8_BAUDRATE);
    sensor_S8 = new S8_UART(S8_serial);

    Serial.println("we have a sensor");

    S8_sensor sensor;
    sensor.co2 = sensor_S8->get_co2();

    // send the value to the display task through the queue
    Serial.print("CO2 value = ");
    Serial.print(sensor.co2);
    Serial.println(" ppm");

    Serial.println("setup done");
}

void loop()
{
    delay(5);
}