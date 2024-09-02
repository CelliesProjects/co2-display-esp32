#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "displayMessageStruct.hpp"

extern void displayTask(void *parameter);
extern QueueHandle_t displayQueue;
static TaskHandle_t displayTaskHandle = nullptr;

extern void sensorTask(void *parameter);
static TaskHandle_t sensorTaskHandle = nullptr;

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    // mount sd card
    SPI.begin(48, 41, 47);
    if (!SD.begin(42, SPI, 20000000))  // might not run/ be slow at 20MHz - check!
        Serial.println("SD card not found");
    else
        Serial.println("SD card mounted");

    displayQueue = xQueueCreate(5, sizeof(struct displayMessage));
    if (!displayQueue)
    {
        log_e("FATAL error! could not create display queue. System HALTED!");
        while (1)
            delay(100);
    }

    auto taskResult = xTaskCreate(displayTask,
                                  NULL,
                                  4096,
                                  NULL,
                                  10,
                                  &displayTaskHandle);

    if (taskResult != pdPASS)
    {
        log_e("FATAL error! Could not create playerTask. System HALTED!");
        while (1)
            delay(100);
    }

    taskResult = xTaskCreate(sensorTask,
                                  NULL,
                                  4096,
                                  NULL,
                                  10,
                                  &sensorTaskHandle);

    if (taskResult != pdPASS)
    {
        log_e("FATAL error! Could not create sensorTask. System HALTED!");
        // send panic to the display
        while (1)
            delay(100);
    }    

}

void loop()
{
    delay(1500);
}