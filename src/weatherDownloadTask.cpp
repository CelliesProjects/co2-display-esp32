#include "weatherDownloadTask.hpp"

void weatherDownloadTask(void *parameter)
{
    if (!WiFi.isConnected() || !VISUAL_CROSSING_CITY || !VISUAL_CROSSING_COUNTRY || !VISUAL_CROSSING_API_KEY)
    {
        log_e("can not start weatherTask because of reasons");
        vTaskDelete(NULL);
    }

    static char url[512];
    const int urlLength = snprintf(url, sizeof(url),
                                   "https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/%s,%%20%s/"
                                   "next24hours?unitGroup=metric&elements=datetimeEpoch,temp,icon&include=current&key=%s"
                                   "&options=nonulls&contentType=json",
                                   VISUAL_CROSSING_CITY,
                                   VISUAL_CROSSING_COUNTRY,
                                   VISUAL_CROSSING_API_KEY);

    if (urlLength < 0 || urlLength >= sizeof(url))
    {
        log_e("URL length exceeds buffer size, unable to proceed");
        vTaskDelete(NULL);
    }

    log_d("request url: %s", url);

    constexpr static const char *queryCost = "queryCost";
    constexpr static const char *currentConditions = "currentConditions";
    constexpr static const char *icon = "icon";
    constexpr static const char *temp = "temp";

    static WiFiClientSecure client;
    static HTTPClient http;

    client.setInsecure();

    constexpr const int TASK_DELAY_MS = 60 * 30 * 1000;

    while (1)
    {
        if (!http.begin(client, url))
        {
            log_e("could not reach %s", url);
            vTaskDelay(pdTICKS_TO_MS(TASK_DELAY_MS));
            continue;
        }

        const auto RESPONSE_CODE = http.GET();
        if (RESPONSE_CODE != HTTP_CODE_OK)
        {
            log_e("could not get weather forecast, error %i\n%s", RESPONSE_CODE, http.errorToString(RESPONSE_CODE).c_str());
            http.end();
            vTaskDelay(pdTICKS_TO_MS(TASK_DELAY_MS));
            continue;
        }

        if (http.getSize() <= 0)
        {
            log_e("Received empty response from server.");
            http.end();
            vTaskDelay(pdTICKS_TO_MS(TASK_DELAY_MS));
            continue;
        }

        // the json filter: it contains "true" for each value we want to keep
        JsonDocument filter;
        filter[queryCost] = true;
        filter[currentConditions] = true;

        JsonDocument doc;
        const DeserializationError JSON_ERROR = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
        if (JSON_ERROR)
        {
            log_e("could not parse JSON because %s", JSON_ERROR.f_str());
            http.end();
            vTaskDelay(pdTICKS_TO_MS(TASK_DELAY_MS));
            continue;
        }

        http.end();

        log_d("query cost %i", int32_t(doc[queryCost]));

        if (doc[currentConditions][icon].isNull() ||
            doc[currentConditions][temp].isNull())
        {
            log_e("missing current condition values, no weather update");
            vTaskDelay(pdTICKS_TO_MS(TASK_DELAY_MS));
            continue;
        }

        showForecast(doc[currentConditions][icon].as<const char *>(), doc[currentConditions][temp]);

        vTaskDelay(pdTICKS_TO_MS(TASK_DELAY_MS));
    }
}
