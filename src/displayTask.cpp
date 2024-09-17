#include "displayTask.hpp"

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// https://lovyangfx.readthedocs.io/en/latest/02_using.html

// https://m5stack.lang-ship.com/howto/m5gfx/font/

void updateCo2History()
{
    auto startMS = millis();

    static LGFX_Sprite co2Graph(&display);
    co2Graph.setColorDepth(lgfx::palette_2bit);
    co2Graph.createSprite(380, 100);
    co2Graph.setPaletteColor(1, 0, 0, 255);
    co2Graph.setPaletteColor(2, 31, 255, 31);
    co2Graph.setPaletteColor(3, 180, 180, 180);

    // grid
    co2Graph.setColor(3);
    co2Graph.setFont(0);
    co2Graph.setTextColor(3);
    co2Graph.setTextDatum(BC_DATUM);
    co2Graph.setTextWrap(false, false);

    const auto LOWEST_LEVEL_PPM = 400;
    const auto HIGHEST_LEVEL_PPM = 2000;

    for (auto hgrid = 500; hgrid < HIGHEST_LEVEL_PPM; hgrid += 500)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, co2Graph.height(), 0);
        co2Graph.writeFastHLine(0, ypos, co2Graph.width());
        co2Graph.setCursor(10, ypos);
        co2Graph.print(hgrid);
        co2Graph.setCursor(co2Graph.width() >> 1, ypos);
        co2Graph.print(hgrid);
        co2Graph.setCursor(co2Graph.width() - 35, ypos);
        co2Graph.print(hgrid);
    }

    // actual data
    const auto BAR_WIDTH = 3;
    const auto GAP_WIDTH = 1;

    auto cnt = 0;
    co2Graph.setColor(2);
    for (const auto &item : history)
    {
        const auto BAR_HEIGHT = mapf(item.co2, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, 0, co2Graph.height());
        const auto xpos = co2Graph.width() - cnt * (BAR_WIDTH + GAP_WIDTH);
        co2Graph.fillRect(xpos - BAR_WIDTH,
                          co2Graph.height(),
                          BAR_WIDTH,
                          -BAR_HEIGHT);
        if (xpos < 0)
            break;
        cnt++;
    }
    Serial.printf("pushed %i items to screen\n", cnt);
    co2Graph.pushSprite(0, 0);

    Serial.printf("done in %lums\n", millis() - startMS);
}

void updateCo2Value(const size_t newValue)
{

    static LGFX_Sprite co2Value(&display);
    co2Value.setColorDepth(lgfx::palette_2bit);
    co2Value.createSprite(100, 100);
    co2Value.setPaletteColor(1, 0, 0, 255);
    co2Value.setPaletteColor(2, 31, 255, 31);
    co2Value.setPaletteColor(3, 180, 180, 180);

    co2Value.fillCircle(co2Value.width() >> 1, co2Value.height() >> 1, 50, 1);
    co2Value.setTextDatum(CC_DATUM);
    co2Value.setTextColor(2);
    co2Value.drawNumber(newValue, co2Value.width() >> 1, (co2Value.height() >> 1) + 4, &DejaVu40);
    //co2Value.setTextColor(0);
    co2Value.drawString("CO", 46, 20, &DejaVu24);
    co2Value.drawString("2", 68, 10, &DejaVu12);
    co2Value.drawString("ppm", 50, 80, &DejaVu18);

    co2Value.pushSprite(380,0);
}

// https://lovyangfx.readthedocs.io/en/latest/02_using.html

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
                updateCo2Value(msg.sizeVal);
                /*
                                display.setCursor(50, 100);
                                display.setTextColor(TFT_BLACK, TFT_WHITE);
                                display.setTextSize(2 , 5 );
                display.printf("CO2 %06i ppm", msg.sizeVal);
                */

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