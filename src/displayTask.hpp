#ifndef DISPLAY_TASK
#define DISPLAY_TASK

#include <Arduino.h>
#include <list>
#include <vector>

#include "storageStruct.hpp"
#include "forecast_t.hpp"
#include "displayMessageStruct.hpp"

#include "LGFX_ESP32S3_RGB_GUITION_480x480_ST7701s_GT911.h"
#include <LovyanGFX.hpp>

#include "fonts/DejaVu24-modded.h" /* contains percent sign ans superscript 2*/
#include "fonts/DejaVu40-modded.h" /* contains percent sign ans superscript 2*/

static const auto BAR_WIDTH = 2; /* the bars inside a graph */
static const auto GAP_WIDTH = 2; /* the gap between the bars */

static const auto GRAPH_WIDTH = 360;
static const auto GRAPH_HEIGHT = 120;
static const auto VALUE_WIDTH = 120;

extern std::list<struct storageStruct> history;
extern void startWeatherTask();

static LGFX display;

static const auto BACKGROUND_COLOR = display.color565(0,0,0);
static const auto GRAPH_BACKGROUND = display.color565(50, 50, 50);
static const auto CLOCK_BACKGROUND = display.color565(245, 102, 10);
static const auto WEATHER_BACKGROUND = display.color565(245, 102, 10);

QueueHandle_t displayQueue = nullptr;

/* weather icons from https://github.com/visualcrossing/WeatherIcons/tree/main/PNG/1st%20Set%20-%20Color */

extern const uint8_t clear_day_png_start[] asm("_binary_src_weather_icons_clear_day_png_start");
extern const uint8_t clear_day_png_end[] asm("_binary_src_weather_icons_clear_day_png_end");

extern const uint8_t clear_night_png_start[] asm("_binary_src_weather_icons_clear_night_png_start");
extern const uint8_t clear_night_png_end[] asm("_binary_src_weather_icons_clear_night_png_end");

extern const uint8_t cloudy_png_start[] asm("_binary_src_weather_icons_cloudy_png_start");
extern const uint8_t cloudy_png_end[] asm("_binary_src_weather_icons_cloudy_png_end");

extern const uint8_t fog_png_start[] asm("_binary_src_weather_icons_fog_png_start");
extern const uint8_t fog_png_end[] asm("_binary_src_weather_icons_fog_png_end");

extern const uint8_t hail_png_start[] asm("_binary_src_weather_icons_hail_png_start");
extern const uint8_t hail_png_end[] asm("_binary_src_weather_icons_hail_png_end");

extern const uint8_t partly_cloudy_day_png_start[] asm("_binary_src_weather_icons_partly_cloudy_day_png_start");
extern const uint8_t partly_cloudy_day_png_end[] asm("_binary_src_weather_icons_partly_cloudy_day_png_end");

extern const uint8_t partly_cloudy_night_png_start[] asm("_binary_src_weather_icons_partly_cloudy_night_png_start");
extern const uint8_t partly_cloudy_night_png_end[] asm("_binary_src_weather_icons_partly_cloudy_night_png_end");

extern const uint8_t rain_png_start[] asm("_binary_src_weather_icons_rain_png_start");
extern const uint8_t rain_png_end[] asm("_binary_src_weather_icons_rain_png_end");

extern const uint8_t rain_snow_png_start[] asm("_binary_src_weather_icons_rain_snow_png_start");
extern const uint8_t rain_snow_png_end[] asm("_binary_src_weather_icons_rain_snow_png_end");

extern const uint8_t rain_snow_showers_day_png_start[] asm("_binary_src_weather_icons_rain_snow_showers_day_png_start");
extern const uint8_t rain_snow_showers_day_png_end[] asm("_binary_src_weather_icons_rain_snow_showers_day_png_end");

extern const uint8_t rain_snow_showers_night_png_start[] asm("_binary_src_weather_icons_rain_snow_showers_night_png_start");
extern const uint8_t rain_snow_showers_night_png_end[] asm("_binary_src_weather_icons_rain_snow_showers_night_png_end");

extern const uint8_t showers_day_png_start[] asm("_binary_src_weather_icons_showers_day_png_start");
extern const uint8_t showers_day_png_end[] asm("_binary_src_weather_icons_showers_day_png_end");

extern const uint8_t showers_night_png_start[] asm("_binary_src_weather_icons_showers_night_png_start");
extern const uint8_t showers_night_png_end[] asm("_binary_src_weather_icons_showers_night_png_end");

extern const uint8_t sleet_png_start[] asm("_binary_src_weather_icons_sleet_png_start");
extern const uint8_t sleet_png_end[] asm("_binary_src_weather_icons_sleet_png_end");

extern const uint8_t snow_png_start[] asm("_binary_src_weather_icons_snow_png_start");
extern const uint8_t snow_png_end[] asm("_binary_src_weather_icons_snow_png_end");

extern const uint8_t snow_showers_day_png_start[] asm("_binary_src_weather_icons_snow_showers_day_png_start");
extern const uint8_t snow_showers_day_png_end[] asm("_binary_src_weather_icons_snow_showers_day_png_end");

extern const uint8_t snow_showers_night_png_start[] asm("_binary_src_weather_icons_snow_showers_night_png_start");
extern const uint8_t snow_showers_night_png_end[] asm("_binary_src_weather_icons_snow_showers_night_png_end");

extern const uint8_t thunder_png_start[] asm("_binary_src_weather_icons_thunder_png_start");
extern const uint8_t thunder_png_end[] asm("_binary_src_weather_icons_thunder_png_end");

extern const uint8_t thunder_rain_png_start[] asm("_binary_src_weather_icons_thunder_rain_png_start");
extern const uint8_t thunder_rain_png_end[] asm("_binary_src_weather_icons_thunder_rain_png_end");

extern const uint8_t thunder_showers_day_png_start[] asm("_binary_src_weather_icons_thunder_showers_day_png_start");
extern const uint8_t thunder_showers_day_png_end[] asm("_binary_src_weather_icons_thunder_showers_day_png_end");

extern const uint8_t thunder_showers_night_png_start[] asm("_binary_src_weather_icons_thunder_showers_night_png_start");
extern const uint8_t thunder_showers_night_png_end[] asm("_binary_src_weather_icons_thunder_showers_night_png_end");

extern const uint8_t wind_png_start[] asm("_binary_src_weather_icons_wind_png_start");
extern const uint8_t wind_png_end[] asm("_binary_src_weather_icons_wind_png_end");

#endif
