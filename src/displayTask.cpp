#include "displayTask.hpp"

long mapl(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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
// https://lovyangfx.readthedocs.io/en/latest/02_using.html

// https://m5stack.lang-ship.com/howto/m5gfx/font/

void updateCo2History()
{
    const auto LOWEST_LEVEL_PPM = 400;

    static LGFX_Sprite co2Graph(&display);
    co2Graph.setColorDepth(lgfx::palette_2bit);
    co2Graph.createSprite(400, 100);
    co2Graph.setPaletteColor(1, 0, 0, 255);
    co2Graph.setPaletteColor(2, 31, 255, 31);
    co2Graph.setPaletteColor(3, 255, 255, 191);

    const auto GAP_WIDTH = 2;
    const auto BAR_WIDTH = 1;

    Serial.printf("starting loop at %lu\n", millis());

    auto itemsNeeded = co2Graph.width() / (BAR_WIDTH + GAP_WIDTH);
    int32_t highestCo2 = 0;
    for (const auto &item : history)
    {
        highestCo2 = std::max(highestCo2, item.co2);
        if (!itemsNeeded--)
            break;
    }
    Serial.printf("found highest at %lu\n", millis());

    // draw a grid to indicate 500ppm and 1000ppm and so on
    // change this to a while loop as it takes +-1700ms this way
    co2Graph.setColor(1);
    for (int hgrid = 500;; hgrid = hgrid + 500)
    {
        const auto ypos = mapl(hgrid, LOWEST_LEVEL_PPM, highestCo2, co2Graph.height(), 0);
        if (ypos > co2Graph.height())
            break;
        //co2Graph.startWrite();
        co2Graph.drawLine(0, ypos, co2Graph.width(), ypos);
        //co2Graph.endWrite();
        // Serial.printf("drawing %i line", hgrid);
        //  co2Graph.setColor(3);
        //  co2Graph.setCursor(co2Graph.width() / 2, ypos);
        //  co2Graph.print(hgrid);
    }

    Serial.printf("grid drawn at %lu\n", millis());

    auto cnt = 0;
    co2Graph.setColor(2);
    for (const auto &item : history)
    {
        const auto BAR_HEIGHT = mapl(item.co2, LOWEST_LEVEL_PPM, highestCo2, 0, co2Graph.height());
        const auto xpos = co2Graph.width() - cnt * (BAR_WIDTH + GAP_WIDTH);
        co2Graph.fillRect(xpos - BAR_WIDTH,
                          co2Graph.height(),
                          BAR_WIDTH,
                          -BAR_HEIGHT);
        if (xpos < 0)
            break;
        cnt++;
    }

    Serial.printf("graph drawn at %lu\n", millis());

    // draw the label on the graph
    co2Graph.setFont(&fonts::Font4);
    co2Graph.setTextDatum(lgfx::bottom_left);
    co2Graph.setColor(0);
    co2Graph.setTextColor(2);
    const char label[] = "CO2 level ppm";
    const auto textWidth = co2Graph.textWidth(label) + 2;
    const auto textHeight = co2Graph.fontHeight();
    co2Graph.fillRect(2, co2Graph.height() - textHeight, textWidth, textHeight);

    co2Graph.drawString(label, 2, co2Graph.height());

    Serial.printf("pushed %i items to screen\n", cnt);
    co2Graph.pushSprite(10, 320);

    Serial.printf("done at %lu\n\n", millis());
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

        int32_t x, y;
        if (display.getTouch(&x, &y))
        {
            display.fillRect(x - 2, y - 2, 5, 5, TFT_BLUE);
        }
    }
}