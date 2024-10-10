#include "weatherDownloadTask.hpp"

void weatherDownloadTask(void *parameter)
{
    if (!WiFi.isConnected() || !VISUAL_CROSSING_CITY || !VISUAL_CROSSING_COUNTRY || !VISUAL_CROSSING_API_KEY)
    {
        log_e("can not start weatherTask because of reasons");
        vTaskDelete(NULL);
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    {
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

    forecasts.clear();

    /* only get current conditions first time after boot */
    static bool firstRun = true; 
    if (firstRun)
    {
        if (doc["currentConditions"]["icon"].isNull() ||
            doc["currentConditions"]["temp"].isNull() ||
            doc["currentConditions"]["datetimeEpoch"].isNull())
        {
            log_e("missing current condition values, aborting.");
            vTaskDelete(NULL);
        }

        forecast_t first{};
        snprintf(first.icon, sizeof(first.icon), "%s", doc["currentConditions"]["icon"].as<const char *>());
        first.temp = doc["currentConditions"]["temp"];
        first.time = doc["currentConditions"]["datetimeEpoch"];
        forecasts.push_back(first);
        firstRun = false;
    }

    const JsonArray hours = doc["days"][0]["hours"].as<JsonArray>();
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

    vTaskDelete(NULL);
}
