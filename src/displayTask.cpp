#include "displayTask.hpp"

void printHelloWorld()
{
    display.setCursor(random(display.width()), random(display.height()));
    display.setTextColor(random(0xffff), random(0xffff));
    display.setTextSize(random(6) /* x scale */, random(6) /* y scale */);
    display.println("Hello World!");
}

void displayTask(void *parameter)
{
    display.init();
    display.setBrightness(90);
    display.setTextSize((std::max(display.width(), display.height()) + 255) >> 8);
    display.fillScreen(TFT_GOLD);

    while (1)
    {
        static struct displayMessage msg;
        if (xQueueReceive(displayQueue, &msg, pdTICKS_TO_MS(25)) == pdTRUE)
        {
            switch (msg.type)
            {
            case displayMessage::SYSTEM_MESSAGE:
            {
                display.setCursor(0, 0);
                display.setTextColor(TFT_BLACK, TFT_WHITE);
                display.setTextSize(2 /* x scale */, 5 /* y scale */);
                display.println(msg.str);
                break;
            }

            case displayMessage::CO2_LEVEL:
            {
                display.setCursor(50, 100);
                display.setTextColor(TFT_BLACK, TFT_WHITE);
                display.setTextSize(2 /* x scale */, 5 /* y scale */);
                display.printf("CO2 %06i ppm", msg.sizeVal);
                break;
            }

            case displayMessage::TEMPERATURE:
            {
                display.setCursor(50, 150);
                display.setTextColor(TFT_BLACK, TFT_WHITE);
                display.setTextSize(2 /* x scale */, 5 /* y scale */);
                display.printf("TEMP: %.1f C", msg.floatVal);
                break;
            }

            case displayMessage::HUMIDITY:
            {
                display.setCursor(50, 200);
                display.setTextColor(TFT_BLACK, TFT_WHITE);
                display.setTextSize(2 /* x scale */, 5 /* y scale */);
                display.printf("HUMIDITY: %i%%", msg.sizeVal);
                break;
            }

            default:
                log_w("unhandled tft msg type");
            }
        }
        int32_t x, y;
        if (display.getTouch(&x, &y))
        {
            display.fillRect(x - 2, y - 2, 5, 5, TFT_BLUE);
        }
    }
}