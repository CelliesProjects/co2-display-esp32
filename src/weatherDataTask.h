#ifndef _WEATHER_DATA_TASK_
#define _WEATHER_DATA_TASK_

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "displayMessageStruct.hpp"
#include "forecast_t.hpp"

extern const char *VISUAL_CROSSING_API_KEY;
extern const char *VISUAL_CROSSING_CITY;
extern const char *VISUAL_CROSSING_COUNTRY;

extern QueueHandle_t displayQueue;

extern std::vector<forecast_t> forecasts;

void getWeatherDataTask(void *parameter);

#endif