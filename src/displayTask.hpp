#ifndef DISPLAY_TASK
#define DISPLAY_TASK

#include <Arduino.h>
// #include <HTTPClient.h>
#include <list>

#include "storageStruct.hpp"
#include "displayMessageStruct.hpp"

#include "LGFX_ESP32S3_RGB_GUITION_480x480_ST7701s_GT911.h"
#include <LovyanGFX.hpp>

#include "fonts/DejaVu24-modded.h" /* contains percent sign ans superscript 2*/

static const auto BAR_WIDTH = 2; /* the bars inside a graph */
static const auto GAP_WIDTH = 2; /* the gap between the bars */

static const auto GRAPH_WIDTH = 370;
static const auto GRAPH_HEIGHT = 110;

extern std::list<struct storageStruct> history;

extern const char *VISUAL_CROSSING_API_KEY;
extern const char *VISUAL_CROSSING_LOCATON;

static LGFX display;

static const auto BACKGROUND_COLOR = display.color565(245, 102, 10);

QueueHandle_t displayQueue = nullptr;

void displayTask(void *parameter);

#endif