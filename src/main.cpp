#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include <list>

#include <WebSocketsClient.h> /* https://github.com/Links2004/arduinoWebSockets */

#define WEBSOCKET_SERVER "192.168.0.20"
#define WEBSOCKET_PORT 80
#define WEBSOCKET_URL "/sensors"
#define WEBSOCKET_TIMEOUT 4000

#include "secrets.h" /* untracked file containing wifi credentials */
#include "storageStruct.hpp"
#include "displayMessageStruct.hpp"

extern void displayTask(void *parameter);
extern QueueHandle_t displayQueue;
static TaskHandle_t displayTaskHandle = nullptr;

static WebSocketsClient webSocket;

static std::list<struct storageStruct> history;

static void buildHistory(char *payload)
{
    char *pch = strtok(payload, "\n");

    if (strstr(pch, "G:") != payload)
    {
        Serial.println("payload is not a history list");
        return;
    }

    pch = strtok(NULL, "\n"); // read first line of history

    while (pch)
    {
        struct storageStruct item = {NAN, 0, 0};

        char *temp = strstr(pch, "T:");
        if (temp)
            item.temp = atof(temp + 2);

        char *humidity = strstr(pch, "H:");
        if (humidity)
            item.humidity = atoi(humidity + 2);

        char *co2 = strstr(pch, "C:");
        if (co2)
            item.co2 = atoi(co2 + 2);

        if (!isnan(item.temp) && item.co2 && item.humidity)
        {
            history.push_back(item);
            //Serial.printf("item added \t%.1fC\t%i%%\t%ippm\n", item.temp, item.humidity, item.co2);
        }
        item = {NAN, 0, 0};

        pch = strtok(NULL, "\n");
    }
}

/*
   we can have the following tags for messages:
   A: new saved average to add to the history
   C: current co2 level
   G: history
   H: current humidity
   P: ping used to keep the server informed that the client is not yet buggered off
   T: current temperature
   timeout is 4 seconds before client reconnects
*/
void processPayload(char *payload)
{
    if (payload[1] != ':')
    {
        Serial.printf("payload contains no : \n%s", payload);
        return;
    }
    switch (payload[0])
    {
    case 'G':
        buildHistory(payload);
        break;
    case 'T':
    {
        displayMessage msg;
        msg.type = displayMessage::TEMPERATURE;
        msg.floatVal = atof(&payload[2]);
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
    }
    break;
    case 'H':
    {
        displayMessage msg;
        msg.type = displayMessage::HUMIDITY;
        msg.sizeVal = atoi(&payload[2]);
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
    }
    break;
    case 'C':
    {
        displayMessage msg;
        msg.type = displayMessage::CO2_LEVEL;
        msg.sizeVal = atoi(&payload[2]);
        xQueueSend(displayQueue, &msg, portMAX_DELAY);
    }
    break;
    default:
        log_e("unknown payload type %c", payload[0]);
    }
}
static auto lastContactMS = millis();

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[WSc] Disconnected!\n");
        break;
    case WStype_CONNECTED:
        Serial.printf("[WSc] Connected to url: %s\n", payload);
        lastContactMS = millis();
        break;
    case WStype_TEXT:
        processPayload((char *)payload);
        lastContactMS = millis();
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

    displayQueue = xQueueCreate(5, sizeof(struct displayMessage));
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
                                  10,
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

    // setup the websocket connection

    webSocket.begin(WEBSOCKET_SERVER, WEBSOCKET_PORT, WEBSOCKET_URL);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(600);
}

void loop()
{
    if (millis() - lastContactMS > WEBSOCKET_TIMEOUT)
    {
        webSocket.disconnect();
        lastContactMS = millis();
    }
    webSocket.loop();
    delay(2);
}