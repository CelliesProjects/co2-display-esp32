#include "weatherDataTask.h"

// https://github.com/visualcrossing/WeatherApi/blob/master/Arduino_samples_esp32/src/sketch.ino

void getWeatherDataTask(void *parameter)
{
    if (!WiFi.isConnected() || !VISUAL_CROSSING_CITY || !VISUAL_CROSSING_COUNTRY || !VISUAL_CROSSING_API_KEY)
    {
        log_e("can not start weatherTask because of reasons");
        vTaskDelete(NULL);
    }

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
        url.concat("?unitGroup=metric&elements=datetime,temp,description,conditions,icon&include=days%2Cobs&key=");
        url.concat(VISUAL_CROSSING_API_KEY);
        url.concat("&contentType=json");

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