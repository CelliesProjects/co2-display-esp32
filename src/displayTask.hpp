#ifndef DISPLAY_TASK
#define DISPLAY_TASK

#include <Arduino.h>
#include <list>

//#include "fonts/Ubuntu-Medium-18.h"

#include "storageStruct.hpp"
#include "displayMessageStruct.hpp"

#include "LGFX_ESP32S3_RGB_GUITION_480x480_ST7701s_GT911.h"
#include <LovyanGFX.hpp>

extern std::list<struct storageStruct> history;

static LGFX display;

QueueHandle_t displayQueue = nullptr;

void displayTask(void *parameter);

#endif