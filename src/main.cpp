#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <list>

#include <WebSocketsClient.h> /* https://github.com/Links2004/arduinoWebSockets */

#define WEBSOCKET_SERVER "192.168.0.20"
#define WEBSOCKET_PORT 80
#define WEBSOCKET_URL "/sensors"
#define WEBSOCKET_TIMEOUT 4000

#define HISTORY_MAX_ITEMS 180

#define DISPLAY_QUEUE_MAX_ITEMS 8

#include "secrets.h" /* untracked file containing wifi credentials */
#include "storageStruct.hpp"
#include "displayMessageStruct.hpp"

extern void displayTask(void *parameter);
extern QueueHandle_t displayQueue;
static TaskHandle_t displayTaskHandle = nullptr;

// https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/protocols/esp_websocket_client.html

// https://github.com/espressif/esp-idf/blob/v4.2/examples/protocols/websocket/main/websocket_example.c

static WebSocketsClient webSocket;
std::list<struct storageStruct> history;

static auto lastWebsocketEventMS = 0;

/*
   these are the tags for messages:
   A: new saved average to add to the history
   C: current co2 level
   G: history
   H: current humidity
   P: ping used to keep the server informed that the client has not yet buggered off
   T: current temperature
   timeout is 4 seconds before client reconnects
*/
static void addItemToHistory(char *payload)
{
    char *pch = strtok(payload, "\n");
    if (strcmp(pch, "A:"))
    {
        Serial.println("not a valid item");
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
        Serial.println("not a history list");
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

void processPayload(char *payload)
{
    if (payload[1] != ':')
    {
        Serial.printf("payload contains no separator ':' \n%s", payload);
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
        Serial.printf("unknown payload type '%c'\n", payload[0]);
    }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[WSc] Disconnected!\n");
        break;
    case WStype_CONNECTED:
        Serial.printf("[WSc] Connected to ws://%s:%i%s\n", WEBSOCKET_SERVER, WEBSOCKET_PORT, WEBSOCKET_URL);
        if (history.empty())
            webSocket.sendTXT("G:\n");
        lastWebsocketEventMS = millis();
        break;
    case WStype_TEXT:
        processPayload((char *)payload);
        lastWebsocketEventMS = millis();
        break;
    case WStype_BIN:
        Serial.printf("[WSc] get binary length: %u\n", length);
        // hexdump(payload, length);

        // send data to server
        // webSocket.sendBIN(payload, length);
        break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.printf("connecting to %s\n", WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PSK);

    // mount sd card
    SPI.begin(48, 41, 47);
    if (!SD.begin(42, SPI, 20000000)) // might not run/ be slow at 20MHz - check!
        Serial.println("SD card not found");
    else
        Serial.println("SD card mounted");

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
        log_e("FATAL error! Could not create playerTask. System HALTED!");
        while (1)
            delay(100);
    }
    Serial.printf("waiting for WiFi network %s to connect\n", WIFI_SSID);

    while (!WiFi.isConnected())
        delay(10);

    Serial.printf("connected to %s\n", WIFI_SSID);

    configTzTime(TIMEZONE, NTP_POOL);

    webSocket.begin(WEBSOCKET_SERVER, WEBSOCKET_PORT, WEBSOCKET_URL);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(600);
}

void loop()
{
    if (webSocket.isConnected() && millis() - lastWebsocketEventMS > WEBSOCKET_TIMEOUT)
    {
        Serial.println("ws timeout");
        webSocket.disconnect();
        lastWebsocketEventMS = millis();
    }
    webSocket.loop();
    yield();
}