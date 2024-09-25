#include "displayTask.hpp"

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// https://m5stack.lang-ship.com/howto/m5gfx/font/                                            <-------font list

// https://lovyangfx.readthedocs.io/en/latest/02_using.html

// https://m5stack.lang-ship.com/howto/m5gfx/font/

// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts

// https://learn.adafruit.com/adafruit-gfx-graphics-library?view=all#extended-characters-cp437-and-a-lurking-bug-3100368

// https://oleddisplay.squix.ch/

// https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Smooth%20Fonts/FLASH_Array/Font_Demo_1_Array/Font_Demo_1_Array.ino

// https://rop.nl/truetype2gfx/

// https://github.com/robjen/GFX_fonts

// https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

static void updateCo2History(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    auto startMS = millis();

    static LGFX_Sprite co2Graph(&display);
    co2Graph.setColorDepth(lgfx::rgb565_2Byte);
    co2Graph.setPsram(true);
    co2Graph.createSprite(w, h);
    co2Graph.setTextSize(1);
    co2Graph.setTextWrap(false, false);

    const auto GREEN_MAX_PPM = 500;
    const auto YELLOW_MAX_PPM = 650;
    const auto RED_MAX_PPM = 1000;

    const auto BAR_WIDTH = 1;
    const auto GAP_WIDTH = 2;

    const auto LOWEST_LEVEL_PPM = 400;
    const auto HIGHEST_LEVEL_PPM = 2000;

    const int GREEN_MAX_Y = mapf(GREEN_MAX_PPM, HIGHEST_LEVEL_PPM, LOWEST_LEVEL_PPM, 0, h);
    const int YELLOW_MAX_Y = mapf(YELLOW_MAX_PPM, HIGHEST_LEVEL_PPM, LOWEST_LEVEL_PPM, 0, h);
    const int RED_MAX_Y = mapf(RED_MAX_PPM, HIGHEST_LEVEL_PPM, LOWEST_LEVEL_PPM, 0, h);

    // a single bar -1 pixel wide- with the required gradients
    LGFX_Sprite bar(&co2Graph);
    bar.setColorDepth(lgfx::rgb565_2Byte);
    bar.createSprite(1, h);

    constexpr const auto GREEN = bar.color565(0, 255, 0);
    constexpr const auto YELLOW = bar.color565(255, 255, 0);
    constexpr const auto RED = bar.color565(255, 0, 0);

    bar.drawLine(0, 0, 0, RED_MAX_Y, RED);
    bar.drawGradientLine(0, RED_MAX_Y, 0, YELLOW_MAX_Y, RED, YELLOW);
    bar.drawGradientLine(0, YELLOW_MAX_Y, 0, GREEN_MAX_Y, YELLOW, GREEN);
    bar.drawLine(0, GREEN_MAX_Y, 0, h, GREEN);

    // now we use the bar to copy to the screen as a sprite or as separate pixels
    auto cnt = 0;
    for (const auto &item : history)
    {
        const int BAR_HEIGHT = mapf(item.co2, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, 0, h);
        const auto xpos = co2Graph.width() - cnt * (BAR_WIDTH + GAP_WIDTH);
        if (BAR_HEIGHT >= h)
            bar.pushSprite(xpos - BAR_WIDTH, 0);
        else
        {
            auto cnt = h - BAR_HEIGHT;
            while (cnt < h)
            {
                co2Graph.drawFastHLine(xpos - BAR_WIDTH, cnt, BAR_WIDTH, bar.readPixel(0, cnt));
                cnt++;
            }
        }
        if (xpos < 0)
            break;
        cnt++;
    }

    // grid
    co2Graph.setTextDatum(CC_DATUM);
    co2Graph.setTextWrap(false, false);
    co2Graph.setTextColor(co2Graph.color565(192, 192, 192), 0);
    co2Graph.startWrite();
    for (auto hgrid = 500; hgrid < HIGHEST_LEVEL_PPM; hgrid += 500)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, co2Graph.height(), 0);
        co2Graph.writeFastHLine(0, ypos, co2Graph.width(), co2Graph.color565(0, 0, 64));
        // co2Graph.drawNumber(hgrid, 20, ypos, &DejaVu12);
        co2Graph.drawNumber(hgrid, co2Graph.width() >> 1, ypos, &DejaVu12);
        // co2Graph.drawNumber(hgrid, co2Graph.width() - 20, ypos, &DejaVu12);
    }

    co2Graph.pushSprite(x, y);
    co2Graph.endWrite();

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
    co2Value.fillScreen(1);

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
    humidityGraph.setPaletteColor(1, 0, 0, 100);
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
    humidityGraph.setTextColor(3, 0);
    humidityGraph.setTextDatum(CC_DATUM);
    humidityGraph.setTextWrap(false, false);

    for (auto hgrid = 25; hgrid < HIGHEST_LEVEL_HUMIDITY; hgrid += 25)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_HUMIDITY, HIGHEST_LEVEL_HUMIDITY, humidityGraph.height(), 0);
        humidityGraph.writeFastHLine(0, ypos, humidityGraph.width(), 1);
        // draw the values in white
        // humidityGraph.drawNumber(hgrid, 20, ypos, &DejaVu12);
        humidityGraph.drawNumber(hgrid, humidityGraph.width() >> 1, ypos, &DejaVu12);
        // humidityGraph.drawNumber(hgrid, humidityGraph.width() - 20, ypos, &DejaVu12);
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
    humidityValue.fillScreen(1);

    humidityValue.setTextDatum(CC_DATUM);
    humidityValue.setTextColor(2);
    humidityValue.drawNumber(newValue, humidityValue.width() >> 1, (humidityValue.height() >> 1) + 4, &DejaVu40);

    const auto xMiddle = humidityValue.width() >> 1;
    humidityValue.drawString("RH", xMiddle, humidityValue.height() >> 2, &DejaVu24);
    humidityValue.drawString("%", xMiddle, humidityValue.height() - (humidityValue.height() >> 2), &DejaVu18);

    humidityValue.pushSprite(x, y);
}

static void updateTempHistory(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    auto startMS = millis();

    static LGFX_Sprite tempGraph(&display);
    tempGraph.setColorDepth(lgfx::rgb565_2Byte);
    tempGraph.setPsram(true);
    tempGraph.createSprite(w, h);
    tempGraph.setTextSize(1);
    tempGraph.setTextWrap(false, false);

    const auto BLUE_MAX_T = 16;
    const auto GREEN_MAX_T = 20;
    const auto YELLOW_MAX_T = 22;
    const auto RED_MAX_T = 25;

    const auto BAR_WIDTH = 1;
    const auto GAP_WIDTH = 2;

    const auto LOWEST_LEVEL_T = 15;
    const auto HIGHEST_LEVEL_T = 28;

    const int BLUE_MAX_Y = mapf(BLUE_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);
    const int GREEN_MAX_Y = mapf(GREEN_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);
    const int YELLOW_MAX_Y = mapf(YELLOW_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);
    const int RED_MAX_Y = mapf(RED_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);

    // a single bar -1 pixel wide- with the required gradients
    LGFX_Sprite bar(&tempGraph);
    bar.setColorDepth(lgfx::rgb565_2Byte);
    bar.createSprite(1, h);

    constexpr const auto BLUE = bar.color565(95, 198, 224);
    constexpr const auto GREEN = bar.color565(0, 255, 0);
    constexpr const auto YELLOW = bar.color565(255, 255, 0);
    constexpr const auto RED = bar.color565(255, 0, 0);

    bar.drawLine(0, 0, 0, RED_MAX_Y, RED);
    bar.drawGradientLine(0, RED_MAX_Y, 0, YELLOW_MAX_Y, RED, YELLOW);
    bar.drawGradientLine(0, YELLOW_MAX_Y, 0, GREEN_MAX_Y, YELLOW, GREEN);
    bar.drawGradientLine(0, GREEN_MAX_Y, 0, BLUE_MAX_Y, GREEN, BLUE);
    bar.drawLine(0, BLUE_MAX_Y, 0, h, BLUE);

    // now we use the bar to copy to the screen as a sprite or as separate pixels
    auto cnt = 0;
    for (const auto &item : history)
    {
        const int BAR_HEIGHT = mapf(item.temp, LOWEST_LEVEL_T, HIGHEST_LEVEL_T, 0, h);
        const auto xpos = tempGraph.width() - cnt * (BAR_WIDTH + GAP_WIDTH);
        if (BAR_HEIGHT >= h)
            bar.pushSprite(xpos - BAR_WIDTH, 0);
        else
        {
            auto cnt = h - BAR_HEIGHT;
            while (cnt < h)
            {
                tempGraph.drawFastHLine(xpos - BAR_WIDTH, cnt, BAR_WIDTH, bar.readPixel(0, cnt));
                cnt++;
            }
        }
        if (xpos < 0)
            break;
        cnt++;
    }

    // grid
    tempGraph.setTextDatum(CC_DATUM);
    tempGraph.setTextWrap(false, false);
    tempGraph.setTextColor(tempGraph.color565(192, 192, 192), 0);
    tempGraph.startWrite();
    for (auto hgrid = 16; hgrid < HIGHEST_LEVEL_T; hgrid += 4)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_T, HIGHEST_LEVEL_T, tempGraph.height(), 0);
        tempGraph.writeFastHLine(0, ypos, tempGraph.width(), tempGraph.color565(0, 0, 64));
        // tempGraph.drawNumber(hgrid, 20, ypos, &DejaVu12);
        tempGraph.drawNumber(hgrid, tempGraph.width() >> 1, ypos, &DejaVu12);
        // tempGraph.drawNumber(hgrid, tempGraph.width() - 20, ypos, &DejaVu12);
    }

    tempGraph.pushSprite(x, y);
    tempGraph.endWrite();

    Serial.printf("pushed %i items to screen\n", cnt);
    Serial.printf("done in %lums\n", millis() - startMS);
}

static void updateTempValue(const int32_t w, const int32_t h, const int32_t x, const int32_t y, float newValue)
{
    static LGFX_Sprite tempValue(&display);
    tempValue.setColorDepth(lgfx::palette_2bit);
    tempValue.createSprite(w, h);
    tempValue.setPaletteColor(1, 0, 0, 255);
    tempValue.setPaletteColor(2, 31, 255, 31);
    tempValue.setPaletteColor(3, 180, 180, 180);
    tempValue.fillScreen(1);

    tempValue.setTextDatum(CC_DATUM);
    tempValue.setTextColor(2);
    tempValue.drawFloat(newValue, 1, tempValue.width() >> 1, (tempValue.height() >> 1) + 4, &DejaVu40);

    const auto xMiddle = tempValue.width() >> 1;
    tempValue.drawString("T", xMiddle, tempValue.height() >> 2, &DejaVu24);

    tempValue.drawString("C", xMiddle, tempValue.height() - (tempValue.height() >> 2), &DejaVu18);
    // tempValue.drawString("°C", xMiddle, tempValue.height() - (tempValue.height() >> 2));

    tempValue.pushSprite(x, y);
}

void displayTask(void *parameter)
{
    display.setColorDepth(lgfx::rgb565_2Byte);
    display.init();
    display.clear(display.color565(0, 0, 255));
    display.setBrightness(130);
    display.setTextWrap(false, false);

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
                updateCo2Value(110, 110, 370, 0, msg.sizeVal);
                break;
            }

            case displayMessage::CO2_HISTORY:
            {
                updateCo2History(370, 110, 0, 0);
                break;
            }

            case displayMessage::TEMPERATURE:
            {
                updateTempValue(110, 110, 370, 240, msg.floatVal);
                break;
            }

            case displayMessage::TEMPERATURE_HISTORY:
            {
                updateTempHistory(370, 110, 0, 240);
                break;
            }

            case displayMessage::HUMIDITY:
            {
                updateHumidityValue(110, 110, 370, 120, msg.sizeVal);
                break;
            }

            case displayMessage::HUMIDITY_HISTORY:
            {
                updateHumidityHistory(370, 110, 0, 120);
                break;
            }

            default:
                log_w("unhandled tft msg type");
            }
        }

        int32_t x, y;
        if (display.getTouch(&x, &y))
            display.fillRect(x - 2, y - 2, 5, 5, TFT_BLUE);

        struct tm timeinfo = {0};
        static struct tm prevTime = {0};
        if (getLocalTime(&timeinfo, 0) && prevTime.tm_sec != timeinfo.tm_sec)
        {
            // TODO: first draw the time 88:88:88 in a very lightcolored font with the background overwrite
            // then draw the current time over that - requires a sprite
            char timestr[16];
            strftime(timestr, sizeof(timestr), "%X", &timeinfo); // https://cplusplus.com/reference/ctime/strftime/
            display.setTextColor(TFT_WHITE, TFT_BLUE);
            display.setTextDatum(CC_DATUM);
            display.setTextSize(2);
            display.drawCentreString(timestr, display.width() >> 1, 365, &Font7);
            prevTime = timeinfo;
        }
    }
}