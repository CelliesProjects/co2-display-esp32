#include "weatherDataTask.h"

void getWeatherDataTask(void *parameter)
{
    if (!WiFi.isConnected() || !VISUAL_CROSSING_CITY || !VISUAL_CROSSING_COUNTRY)
    {
        log_e("can not start weatherTask because of reasons");
        vTaskDelete(NULL);
    }

    String url;
    url.reserve(256);
    url.concat("https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/");
    url.concat(VISUAL_CROSSING_CITY);
    url.concat(",%20");
    url.concat(VISUAL_CROSSING_COUNTRY);
    url.concat("?unitGroup=metric&elements=datetime,temp,description,conditions,icon&include=days%2Cobs&key=");
    url.concat(VISUAL_CROSSING_API_KEY);
    url.concat("&contentType=json");

    log_v("url: %s", url.c_str());

    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    if (!http.begin(client, url))
    {
        log_e("could not reach %s", url.c_str());
        vTaskDelete(NULL);
    }

    const auto httpResponseCode = http.GET();
    if (httpResponseCode != HTTP_CODE_OK)
    {
        log_e("could not get weather forecast error %i\n%s", httpResponseCode, http.errorToString(httpResponseCode).c_str());
        http.end();
        vTaskDelete(NULL);
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, http.getStream());
    if (error)
    {
        log_e("could not parse JSON because %s", error.f_str());
        http.end();
        vTaskDelete(NULL);
    }

    const char *condition = doc["days"][0]["conditions"];
    float temperature = doc["days"][0]["temp"];

    log_i("condition: %s", condition);
    log_i("temp: %f", temperature);

    http.end();
    vTaskDelete(NULL);
}