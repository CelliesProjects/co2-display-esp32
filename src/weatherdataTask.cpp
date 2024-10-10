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

    // https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/Wageningen%2C%20Holland/today?unitGroup=metric&elements=datetimeEpoch%2Ctemp%2Cdescription%2Cicon&include=hours%2Cfcst%2Cstatsfcst%2Cobs%2Cstats%2Cremote&key=YOUR_API_KEY&options=nonulls&contentType=json

    HTTPClient http;
    {
        // See https://www.visualcrossing.com/weather/weather-data-services/ to compose a sample query
        String url;
        url.reserve(512);
        url.concat("https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/");
        url.concat(VISUAL_CROSSING_CITY);
        url.concat(",%20");
        url.concat(VISUAL_CROSSING_COUNTRY);
        url.concat("/next24hours?unitGroup=metric&elements=datetime,datetimeEpoch,temp,icon&include=current,hours&key=");
        url.concat(VISUAL_CROSSING_API_KEY);
        url.concat("&options=nonulls&contentType=json");

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

    // the json filter: it contains "true" for each value we want to keep
    JsonDocument filter;
    filter["queryCost"] = true;
    filter["days"][0]["hours"] = true;
    filter["currentConditions"] = true;

    JsonDocument doc;
    const auto JSON_ERROR = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    if (JSON_ERROR)
    {
        log_e("could not parse JSON because %s", JSON_ERROR.f_str());
        http.end();
        vTaskDelete(NULL);
    }
    http.end();

    log_i("query cost %i", int32_t(doc["queryCost"]));

    //serializeJsonPretty(doc, Serial);

    // get the current conditions and put them in forecasts

    // const JsonArray cc = doc["currentConditions"].as<JsonArray>();

    forecast_t first{};
    const char *str = doc["currentConditions"]["icon"].as<const char *>();
    //log_i("%s", str);
    snprintf(first.icon, sizeof(first.icon), "%s", str);
    first.temp = doc["currentConditions"]["temp"];
    first.time = doc["currentConditions"]["datetimeEpoch"];
    log_i("current data - temp: %.1f\ttime: %i\ticon:%s", first.temp, first.time, first.icon);

    // now iterate over the values and add them to the vector

    const JsonArray hours = doc["days"][0]["hours"].as<JsonArray>();

    // serializeJsonPretty(hours, Serial);

    forecasts.clear();

    const JsonArray currentConditions = doc["currentConditions"].as<JsonArray>();

    serializeJsonPretty(currentConditions, Serial);

    for (auto const &item : hours)
    {
        forecast_t weather{};

        const char *datetimeStr = "datetimeEpoch";
        weather.time = item[datetimeStr].isNull() ? 0 : item[datetimeStr].as<time_t>();

        if (weather.time <= time(NULL))
            continue;

        const char *tempStr = "temp";
        weather.temp = item[tempStr].isNull() ? NAN : item[tempStr].as<float>();

        const char *iconStr = "icon";
        snprintf(weather.icon, sizeof(weather.icon), "%s", item[iconStr].isNull() ? "" : item[iconStr].as<const char *>());

        forecasts.push_back(weather);
    }
    log_i("%i items imported in weather forecast", forecasts.size());

    //////////////////////  old handler below

    if (doc["days"][0]["icon"].isNull() || doc["days"][0]["temp"].isNull())
    {
        log_w("no values in weather forecast!");
        vTaskDelete(NULL);
    }

    //  https://arduinojson.org/v6/example/filter/

    // lees de T en ICON van de json uit en voeg toe aan vector

    displayMessage msg;
    // const char *condition = doc["days"][0]["conditions"];
    snprintf(msg.str, sizeof(msg.str), "%s", (const char *)doc["days"][0]["icon"]);
    msg.floatVal = doc["days"][0]["temp"];
    msg.type = displayMessage::WEATHER_UPDATE;
    xQueueSend(displayQueue, &msg, portMAX_DELAY);

    vTaskDelete(NULL);
}
