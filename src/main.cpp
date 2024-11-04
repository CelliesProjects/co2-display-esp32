#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <esp_sntp.h>
#include <ESPmDNS.h>
#include <list>

#include <WebSocketsClient.h> /* https://github.com/Links2004/arduinoWebSockets */

#define WEBSOCKET_MDNS_HOSTNAME "sensorhub"
#define WEBSOCKET_PORT 80
#define WEBSOCKET_URL "/sensors"
#define WEBSOCKET_TIMEOUT 4000

#define HISTORY_MAX_ITEMS 180
#define DISPLAY_QUEUE_MAX_ITEMS 8

#include "secrets.h" /* untracked file containing wifi credentials */
#include "storageStruct.hpp"
#include "forecast_t.hpp"
#include "displayMessageStruct.hpp"

extern void displayTask(void *parameter);
extern bool TFTtouched(int32_t &x, int32_t &y);
extern void weatherDownloadTask(void *parameter);

extern QueueHandle_t displayQueue;
static TaskHandle_t displayTaskHandle = nullptr;

static IPAddress websocketServerIP;
static WebSocketsClient webSocket;
static auto lastWebsocketEventMS = 0;

std::list<struct storageStruct> history;

void startWeatherTask()
{
    static TaskHandle_t weathertaskHandle = NULL;
    if (weathertaskHandle && (eTaskGetState(weathertaskHandle) == eRunning))
    {
        log_e("can not start weatherTask - task still running");
        return;
    }
    const auto taskResult = xTaskCreate(weatherDownloadTask,
                                        NULL,
                                        4096 * 2,
                                        NULL,
                                        tskIDLE_PRIORITY + 2,
                                        &weathertaskHandle);
    if (taskResult != pdPASS)
        log_e("Could not create weatherTask");
}

static void addItemToHistory(char *payload)
{
    char *pch = strtok(payload, "\n");
    if (strcmp(pch, "A:"))
    {
        log_e("not a valid item");
        return;
    }

    struct storageStruct item = {NAN, 0, 0};
    pch = strtok(NULL, "\n");
    if (pch)
    {
        const char *temp = strstr(pch, "T:");
        if (temp)
            item.temp = atof(temp + 2);

        const char *humidity = strstr(pch, "H:");
        if (humidity)
            item.humidity = atoi(humidity + 2);

        const char *co2 = strstr(pch, "C:");
        if (co2)
            item.co2 = atoi(co2 + 2);

        if (!isnan(item.temp) && item.co2 && item.humidity)
        {
            if (history.size() == HISTORY_MAX_ITEMS)
                history.pop_back();

            history.push_front(item);
        }
    }
}

static void parseAndBuildHistory(char *payload)
{
    char *pch = strtok(payload, "\n");
    if (strcmp(pch, "G:"))
    {
        log_e("not a history list");
        return;
    }

    history.clear();

    auto cnt = 0;
    struct storageStruct item = {NAN, 0, 0};
    pch = strtok(NULL, "\n");
    while (pch)
    {
        if (history.size() == HISTORY_MAX_ITEMS)
            return;

        const char *temp = strstr(pch, "T:");
        if (temp)
            item.temp = atof(temp + 2);

        const char *humidity = strstr(pch, "H:");
        if (humidity)
            item.humidity = atoi(humidity + 2);

        const char *co2 = strstr(pch, "C:");
        if (co2)
            item.co2 = atoi(co2 + 2);

        if (!isnan(item.temp) && item.co2 && item.humidity)
        {
            history.push_back(item);
            cnt++;
        }
        item = {NAN, 0, 0};

        pch = strtok(NULL, "\n");
    }
}

static void processPayload(char *payload)
{
    if (payload[1] != ':')
    {
        log_e("payload contains no separator ':' \n%s", payload);
        return;
    }

    if (payload[0] == 'P') /* P: used as a ping */
        return;

    switch (payload[0])
    {
    case 'A': /* latest average - single item to add to front of history list */
    {
        addItemToHistory(payload);
        displayMessage msg;
        msg.type = displayMessage::CO2_HISTORY;
        xQueueSend(displayQueue, &msg, portMAX_DELAY);

        msg.type = displayMessage::HUMIDITY_HISTORY;
        xQueueSend(displayQueue, &msg, portMAX_DELAY);

        msg.type = displayMessage::TEMPERATURE_HISTORY;
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
        break;
    }
    case 'G': /* history -sent once after boot to fill the history */
    {
        parseAndBuildHistory(payload);
        displayMessage msg;
        msg.type = displayMessage::CO2_HISTORY;
        xQueueSend(displayQueue, &msg, portMAX_DELAY);

        msg.type = displayMessage::HUMIDITY_HISTORY;
        xQueueSend(displayQueue, &msg, portMAX_DELAY);

        msg.type = displayMessage::TEMPERATURE_HISTORY;
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
        break;
    }

    case 'C': /* current co2 level*/
    {
        displayMessage msg;
        msg.type = displayMessage::CO2_LEVEL;
        msg.sizeVal = atoi(&payload[2]);
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
        break;
    }

    case 'H': /* current humidity */
    {
        displayMessage msg;
        msg.type = displayMessage::HUMIDITY;
        msg.sizeVal = atoi(&payload[2]);
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
        break;
    }

    case 'T': /* current temperature */
    {
        displayMessage msg;
        msg.type = displayMessage::TEMPERATURE;
        msg.floatVal = atof(&payload[2]);
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
        break;
    }

    default:
        log_w("unknown payload type '%c'", payload[0]);
    }
}

static void messageOnTFT(const char *str)
{
    displayMessage msg;
    msg.type = displayMessage::SYSTEM_MESSAGE;
    snprintf(msg.str, sizeof(msg.str), str);
    xQueueSend(displayQueue, &msg, portMAX_DELAY);
}

static void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        log_i("[WSc] Disconnected!");
        break;
    case WStype_CONNECTED:
        log_i("[WSc] Connected to ws://%s:%i%s", websocketServerIP.toString().c_str(), WEBSOCKET_PORT, WEBSOCKET_URL);
        if (history.empty())
            webSocket.sendTXT("G:\n");
        lastWebsocketEventMS = millis();
        break;
    case WStype_TEXT:
        processPayload(reinterpret_cast<char *>(payload));
        lastWebsocketEventMS = millis();
        break;
    case WStype_BIN:
        log_d("[WSc] get binary length: %u", length);
        // hexdump(payload, length);

        // send data to server
        // webSocket.sendBIN(payload, length);
        break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    case WStype_PING:
    case WStype_PONG:
        break;
    }
}

static void ntpSynced(void *cb_arg)
{
    startWeatherTask();
    sntp_set_time_sync_notification_cb(NULL);
}

static bool getSensorhubIP(IPAddress &hubIP)
{
    int ndx = MDNS.queryService("http", "tcp");
    while (--ndx != -1)
    {
        if (MDNS.hostname(ndx) == WEBSOCKET_MDNS_HOSTNAME)
            break;
    }
    if (ndx == -1)
        return false;

    hubIP = MDNS.IP(ndx);
    return true;
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    log_i("connecting to %s", WIFI_SSID);

    WiFi.setSleep(false);
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    // mount sd card
    SPI.begin(48, 41, 47);
    if (!SD.begin(42, SPI, 20000000)) // might not run/ be slow at 20MHz - check!
        log_i("SD card not found");
    else
        log_w("SD card mounted");

    displayQueue = xQueueCreate(DISPLAY_QUEUE_MAX_ITEMS, sizeof(struct displayMessage));
    if (!displayQueue)
    {
        log_e("FATAL error! could not create display queue. System HALTED!");
        while (1)
            delay(100);
    }

    auto taskResult = xTaskCreate(displayTask,
                                  NULL,
                                  4096,
                                  NULL,
                                  tskIDLE_PRIORITY + 1,
                                  &displayTaskHandle);

    if (taskResult != pdPASS)
    {
        log_e("FATAL error! Could not create display task. System HALTED!");
        while (1)
            delay(100);
    }

    messageOnTFT("Looking for WiFi...\nPlease wait");

    log_i("waiting for WiFi network %s to connect", WIFI_SSID);

    while (!WiFi.isConnected())
        vTaskDelay(pdMS_TO_TICKS(10));

    log_i("connected to %s", WIFI_SSID);

    if (mdns_init() != ESP_OK)
    {
        messageOnTFT("mDNS init ERROR. System halted");
        while (1)
            delay(100);
    }

    sntp_set_time_sync_notification_cb((sntp_sync_time_cb_t)ntpSynced);
    configTzTime(TIMEZONE, NTP_POOL);

    constexpr static char searchMess[] = {"Searching the sensors..."};
    messageOnTFT(searchMess);
    while (!getSensorhubIP(websocketServerIP))
    {
        messageOnTFT("No sensors found\nCheck if the sensors are running\nTap the screen to search again");
        int32_t x, y;
        while (!TFTtouched(x, y))
            vTaskDelay(pdTICKS_TO_MS(10));
        messageOnTFT(searchMess);
    }

    webSocket.begin(websocketServerIP, WEBSOCKET_PORT, WEBSOCKET_URL);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(600);
}

constexpr const auto TICK_RATE_HZ = 50;
constexpr const TickType_t ticksToWait = pdTICKS_TO_MS(1000 / TICK_RATE_HZ);
static TickType_t xLastWakeTime = xTaskGetTickCount();

void loop()
{
    vTaskDelayUntil(&xLastWakeTime, ticksToWait);

    if (webSocket.isConnected() && millis() - lastWebsocketEventMS > WEBSOCKET_TIMEOUT)
    {
        log_i("ws timeout");
        webSocket.disconnect();
        lastWebsocketEventMS = millis();
    }
    webSocket.loop();
}
