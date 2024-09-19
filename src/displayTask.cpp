#include "displayTask.hpp"

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// https://lovyangfx.readthedocs.io/en/latest/02_using.html

// https://m5stack.lang-ship.com/howto/m5gfx/font/

static void updateCo2History(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    auto startMS = millis();

    static LGFX_Sprite co2Graph(&display);
    co2Graph.setColorDepth(lgfx::palette_2bit);
    co2Graph.createSprite(w, h);
    co2Graph.setPaletteColor(1, 0, 0, 255);
    co2Graph.setPaletteColor(2, 31, 255, 31);
    co2Graph.setPaletteColor(3, 180, 180, 180);

    const auto BAR_WIDTH = 1;
    const auto GAP_WIDTH = 2;

    const auto LOWEST_LEVEL_PPM = 400;
    const auto HIGHEST_LEVEL_PPM = 2000;

    // actual data
    auto cnt = 0;
    co2Graph.setColor(2); // here we could set the palette color based on the current co2 level
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

    // grid
    co2Graph.setTextColor(3);
    co2Graph.setTextDatum(BC_DATUM);
    co2Graph.setTextWrap(false, false);

    for (auto hgrid = 500; hgrid < HIGHEST_LEVEL_PPM; hgrid += 500)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, co2Graph.height(), 0);
        co2Graph.writeFastHLine(0, ypos, co2Graph.width(), 1);
         // draw the values in white
        co2Graph.drawNumber(hgrid, 20, ypos, &DejaVu12);
        co2Graph.drawNumber(hgrid, co2Graph.width() >> 1, ypos, &DejaVu12);
        co2Graph.drawNumber(hgrid, co2Graph.width() - 20, ypos, &DejaVu12);
    }

    co2Graph.pushSprite(x, y);
    Serial.printf("pushed %i items to screen\n", cnt);
    Serial.printf("done in %lums\n", millis() - startMS);
}

static void updateCo2Value(const int32_t w, const int32_t h, const int32_t x, const int32_t y, size_t newValue)
{
    static LGFX_Sprite co2Value(&display);
    co2Value.setColorDepth(lgfx::palette_2bit);
    co2Value.createSprite(w, h);
    co2Value.setPaletteColor(1, 0, 0, 255);
    co2Value.setPaletteColor(2, 31, 255, 31);
    co2Value.setPaletteColor(3, 180, 180, 180);

    co2Value.fillCircle(co2Value.width() >> 1, co2Value.height() >> 1, w / 2, 1);
    co2Value.setTextDatum(CC_DATUM);
    co2Value.setTextColor(2);
    co2Value.drawNumber(newValue, co2Value.width() >> 1, (co2Value.height() >> 1) + 4, &DejaVu40);

    const auto xMiddle = co2Value.width() >> 1;
    const auto superScriptOffset = 17;

    co2Value.drawString("CO", xMiddle - 5, co2Value.height() >> 2, &DejaVu24);
    co2Value.drawString("2", xMiddle + superScriptOffset, (co2Value.height() >> 2) - 10, &DejaVu12);
    co2Value.drawString("ppm", xMiddle, co2Value.height() - (co2Value.height() >> 2), &DejaVu18);

    co2Value.pushSprite(x, y);
}

static void updateHumidityHistory(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    auto startMS = millis();

    static LGFX_Sprite humidityGraph(&display);
    humidityGraph.setColorDepth(lgfx::palette_2bit);
    humidityGraph.createSprite(w, h);
    humidityGraph.setPaletteColor(1, 0, 0, 255);
    humidityGraph.setPaletteColor(2, 31, 255, 31);
    humidityGraph.setPaletteColor(3, 180, 180, 180);

    const auto BAR_WIDTH = 1;
    const auto GAP_WIDTH = 2;

    const auto LOWEST_LEVEL_HUMIDITY = 0;
    const auto HIGHEST_LEVEL_HUMIDITY = 100;

    // actual data
    auto cnt = 0;
    humidityGraph.setColor(2);
    for (const auto &item : history)
    {
        const auto BAR_HEIGHT = mapf(item.humidity, LOWEST_LEVEL_HUMIDITY, HIGHEST_LEVEL_HUMIDITY, 0, humidityGraph.height());
        const auto xpos = humidityGraph.width() - cnt * (BAR_WIDTH + GAP_WIDTH);
        humidityGraph.fillRect(xpos - BAR_WIDTH,
                          humidityGraph.height(),
                          BAR_WIDTH,
                          -BAR_HEIGHT);
        if (xpos < 0)
            break;
        cnt++;
    }

    // grid
    humidityGraph.setTextColor(3);
    humidityGraph.setTextDatum(BC_DATUM);
    humidityGraph.setTextWrap(false, false);

    for (auto hgrid = 25; hgrid < HIGHEST_LEVEL_HUMIDITY; hgrid += 25)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_HUMIDITY, HIGHEST_LEVEL_HUMIDITY, humidityGraph.height(), 0);
        humidityGraph.writeFastHLine(0, ypos, humidityGraph.width(), 1);
         // draw the values in white
        humidityGraph.drawNumber(hgrid, 20, ypos, &DejaVu12);
        humidityGraph.drawNumber(hgrid, humidityGraph.width() >> 1, ypos, &DejaVu12);
        humidityGraph.drawNumber(hgrid, humidityGraph.width() - 20, ypos, &DejaVu12);
    }

    humidityGraph.pushSprite(x, y);
    Serial.printf("pushed %i items to screen\n", cnt);
    Serial.printf("done in %lums\n", millis() - startMS);
}

static void updateHumidityValue(const int32_t w, const int32_t h, const int32_t x, const int32_t y, size_t newValue)
{
    static LGFX_Sprite humidityValue(&display);
    humidityValue.setColorDepth(lgfx::palette_2bit);
    humidityValue.createSprite(w, h);
    humidityValue.setPaletteColor(1, 0, 0, 255);
    humidityValue.setPaletteColor(2, 31, 255, 31);
    humidityValue.setPaletteColor(3, 180, 180, 180);

    humidityValue.fillCircle(humidityValue.width() >> 1, humidityValue.height() >> 1, w / 2, 1);
    humidityValue.setTextDatum(CC_DATUM);
    humidityValue.setTextColor(2);
    humidityValue.drawNumber(newValue, humidityValue.width() >> 1, (humidityValue.height() >> 1) + 4, &DejaVu40);

    const auto xMiddle = humidityValue.width() >> 1;
    humidityValue.drawString("RH", xMiddle, humidityValue.height() >> 2, &DejaVu24);
    humidityValue.drawString("%", xMiddle, humidityValue.height() - (humidityValue.height() >> 2), &DejaVu18);

    humidityValue.pushSprite(x, y);
}


// https://lovyangfx.readthedocs.io/en/latest/02_using.html

void displayTask(void *parameter)
{
    display.init();
    display.setBrightness(130);

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
                updateCo2Value(110, 110, 0, 0, msg.sizeVal);
                break;
            }

            case displayMessage::TEMPERATURE:
            {
                display.setCursor(50, 350);
                display.setTextColor(TFT_BLACK, TFT_WHITE);
                display.setTextSize(2 /* x scale */, 5 /* y scale */);
                display.printf("TEMP: %.1f C", msg.floatVal);
                break;
            }

            case displayMessage::HUMIDITY:
            {
                updateHumidityValue(110, 110, 0, 110, msg.sizeVal);
                break;
            }

            case displayMessage::HUMIDITY_HISTORY:
            {
                updateHumidityHistory(370, 110, 110, 110);
                break;
            }            

            case displayMessage::CO2_HISTORY:
            {
                updateCo2History(370, 110, 110, 0);
                break;
            }

            default:
                log_w("unhandled tft msg type");
            }
        }

        int32_t x, y;
        if (display.getTouch(&x, &y))
            display.fillRect(x - 2, y - 2, 5, 5, TFT_BLUE);
    }
}