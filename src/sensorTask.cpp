#include "sensorTask.hpp"

void sensorTask(void *parameter)
{
    Serial.println("Setting Second I2C pins");

    // set second I2C interface pins
    I2Csensor.setPins(PIN_SENSOR_SDA, PIN_SENSOR_SCL);

    Serial.println("Starting sht31");

    bool result = sht31.begin(SHT31_DEFAULT_ADDR);
    Serial.printf("sht31 sensor is %s\n", result ? "found" : "absent");

    delay(1000);
    Serial.println("Closing serial - logging continues on display");
    Serial.flush();
    Serial.end();

    // start SenseAir S8 sensor on TXD 43 and RXD 44
    S8_serial.setPins(PIN_S8_RXD, PIN_S8_TXD);
    S8_serial.begin(S8_BAUDRATE);
    sensor_S8 = new S8_UART(S8_serial);

    if (!sensor_S8)
    {
        // log to the display something is wrong and
        // halt and catch fire
    }

    // log to the display that the sensor is working

    while (1)
    {
        delay(1000);
    }
}