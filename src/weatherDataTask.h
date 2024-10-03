#ifndef _WEATHER_DATA_TASK_
#define _WEATHER_DATA_TASK_

#include <HTTPClient.h>
#include <ArduinoJson.h>

extern const char *VISUAL_CROSSING_API_KEY;
extern const char *VISUAL_CROSSING_CITY;
extern const char *VISUAL_CROSSING_COUNTRY;

void getWeatherDataTask(void *parameter);

#endif