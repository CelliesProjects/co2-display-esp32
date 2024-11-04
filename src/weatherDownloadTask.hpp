#ifndef _WEATHER_DATA_TASK_
#define _WEATHER_DATA_TASK_

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "displayMessageStruct.hpp"

extern const char *VISUAL_CROSSING_API_KEY;
extern const char *VISUAL_CROSSING_CITY;
extern const char *VISUAL_CROSSING_COUNTRY;

extern void showForecast(const char *icon, const float temp);

extern QueueHandle_t displayQueue;

void weatherDownloadTask(void *parameter);

#endif
