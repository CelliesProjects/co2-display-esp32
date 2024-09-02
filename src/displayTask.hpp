#ifndef DISPLAY_TASK
#define DISPLAY_TASK

#include <Arduino.h>
#include <Wire.h>

#define PIN_TOUCH_SCL 45
#define PIN_TOUCH_SDA 19
#define PIN_TOUCH_INT 1
#define PIN_TOUCH_RES 2
#define GFX_BL 38

#include "displayMessageStruct.hpp"

#include "LGFX_ESP32S3_RGB_GUITION_480x480_ST7701s_GT911.h"
#include <LovyanGFX.hpp>

static LGFX display;

void displayTask(void *parameter);

QueueHandle_t displayQueue = nullptr;

#endif