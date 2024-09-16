#include "displayTask.hpp"

void helloWorld()
{
    // teken de grafiek in een sprite
    static LGFX_Sprite canvas(&display);

    canvas.setColorDepth(lgfx::palette_2bit);
    canvas.createSprite(400, 100);
    // static int x, y;
    canvas.setPaletteColor(1, 0, 0, 255);
    canvas.setPaletteColor(2, 31, 255, 31);
    canvas.setPaletteColor(3, 255, 255, 191);

    canvas.fillCircle(100, (millis() / 15) % 100, 34, 1);
    canvas.setFont(&fonts::Font4);
    canvas.setTextDatum(lgfx::middle_center);
    canvas.setCursor(10, 20);
    canvas.setTextColor(2);
    canvas.print("hello world!");
    canvas.setTextColor(3);

    canvas.drawString("Hello cruel world!", 100, 50);

    // canvas.fillCircle(100, 50, 7, 2);
    canvas.pushSprite(10, 350);
}

static LGFX_Sprite co2Graph(&display);

void updateCo2History()
{
    co2Graph.setColorDepth(lgfx::palette_2bit);
    co2Graph.createSprite(400, 100);
    co2Graph.setPaletteColor(1, 0, 0, 255);
    co2Graph.setPaletteColor(2, 31, 255, 31);
    co2Graph.setPaletteColor(3, 255, 255, 191);
    co2Graph.setColor(2);

    int32_t highestC02 = 0;
    for (const auto &item : history)    
        highestC02 = std::max(highestC02, item.co2);
    
    auto cnt = 0;
    for (const auto &item : history)
    {
        const auto GAP_WIDTH = 1;
        const auto BAR_WIDTH = 3;
        const auto BAR_HEIGHT = map(item.co2, 0, highestC02, 0, co2Graph.height());
        const auto xpos = co2Graph.width() - cnt * (BAR_WIDTH + GAP_WIDTH);
        co2Graph.drawRect(xpos - BAR_WIDTH,
                          co2Graph.height(),
                          BAR_WIDTH,
                          -BAR_HEIGHT);

        if (xpos < 0)
            break;
        cnt++;
    }
    Serial.printf("pushed %i items to fill screen\n", cnt);
    co2Graph.pushSprite(10, 350);
}

void displayTask(void *parameter)
{
    display.init();
    display.setBrightness(70);
    display.setTextSize((std::max(display.width(), display.height()) + 255) >> 8);
    display.fillScreen(TFT_GOLD);

    while (1)
    {
        static struct displayMessage msg;
        if (xQueueReceive(displayQueue, &msg, pdTICKS_TO_MS(5)) == pdTRUE)
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

            case displayMessage::CO2_HISTORY:
            {
                updateCo2History();
                break;
            }

            default:
                log_w("unhandled tft msg type");
            }
        }
        // updateCo2History();
        int32_t x, y;
        if (display.getTouch(&x, &y))
        {
            display.fillRect(x - 2, y - 2, 5, 5, TFT_BLUE);
        }
    }
}