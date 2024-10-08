#include "weatherDataTask.h"

/* for debug: set to true to get values without using any visualcrossing credits */
#define USE_RANDOM_GENERATED_VALUES false

#if (USE_RANDOM_GENERATED_VALUES == true)
static const char *getRandomWeatherString()
{
    // List of all possible weather strings
    const char *weatherTypes[] = {
        "clear-day", "clear-night", "cloudy", "fog", "hail",
        "partly-cloudy-day", "partly-cloudy-night", "rain", "rain-snow",
        "rain-snow-showers-day", "rain-snow-showers-night", "showers-day",
        "showers-night", "sleet", "snow", "snow-showers-day",
        "snow-showers-night", "thunder", "thunder-rain",
        "thunder-showers-day", "thunder-showers-night", "wind"};

    int numWeatherTypes = sizeof(weatherTypes) / sizeof(weatherTypes[0]);
    randomSeed(esp_random()); // Use ESP32 random seed
    int randomIndex = random(numWeatherTypes);
    return weatherTypes[randomIndex];
}

static float randomFloat(float minVal, float maxVal)
{
    // Convert random integer to float by scaling and shifting
    return minVal + (maxVal - minVal) * (random(10000) / 10000.0);
}
#endif

// https://github.com/visualcrossing/WeatherApi/blob/master/Arduino_samples_esp32/src/sketch.ino

void getWeatherDataTask(void *parameter)
{
    if (!WiFi.isConnected() || !VISUAL_CROSSING_CITY || !VISUAL_CROSSING_COUNTRY || !VISUAL_CROSSING_API_KEY)
    {
        log_e("can not start weatherTask because of reasons");
        vTaskDelete(NULL);
    }

#if (USE_RANDOM_GENERATED_VALUES == true)
    {
        displayMessage msg;
        // const char *condition = doc["days"][0]["conditions"];
        snprintf(msg.str, sizeof(msg.str), "%s", getRandomWeatherString());
        msg.floatVal = randomFloat(-4, 34);
        msg.type = displayMessage::WEATHER_UPDATE;
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
        vTaskDelete(NULL);
    }
#endif

    WiFiClientSecure client;
    client.setInsecure(); // TODO: add root certs

    HTTPClient http;
    {
        // See https://www.visualcrossing.com/weather/weather-data-services/ to compose a sample query
        String url;
        url.reserve(512);
        url.concat("https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/");
        url.concat(VISUAL_CROSSING_CITY);
        url.concat(",%20");
        url.concat(VISUAL_CROSSING_COUNTRY);
        url.concat("/today?unitGroup=metric&elements=datetime,temp,description,conditions,icon&include=days,obs&key=");
        url.concat(VISUAL_CROSSING_API_KEY);
        url.concat("&contentType=json");

        log_v("request url: %s", url.c_str());

        if (!http.begin(client, url))
        {
            log_e("could not reach %s", url.c_str());
            vTaskDelete(NULL);
        }
    }

    const auto RESPONSE_CODE = http.GET();
    if (RESPONSE_CODE != HTTP_CODE_OK)
    {
        log_e("could not get weather forecast, error %i\n%s", RESPONSE_CODE, http.errorToString(RESPONSE_CODE).c_str());
        http.end();
        vTaskDelete(NULL);
    }

    JsonDocument doc;
    const auto JSON_ERROR = deserializeJson(doc, http.getStream());
    if (JSON_ERROR)
    {
        log_e("could not parse JSON because %s", JSON_ERROR.f_str());
        http.end();
        vTaskDelete(NULL);
    }

    http.end();

    log_i("query cost %i", int32_t(doc["queryCost"]));

    if (doc["days"][0]["icon"].isNull() || doc["days"][0]["temp"].isNull())
    {
        log_w("no values in weather forecast!");
        vTaskDelete(NULL);
    }

    displayMessage msg;
    // const char *condition = doc["days"][0]["conditions"];
    snprintf(msg.str, sizeof(msg.str), "%s", (const char *)doc["days"][0]["icon"]);
    msg.floatVal = doc["days"][0]["temp"];
    msg.type = displayMessage::WEATHER_UPDATE;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);

    vTaskDelete(NULL);
}
