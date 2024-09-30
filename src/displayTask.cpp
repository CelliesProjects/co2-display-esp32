#include "displayTask.hpp"

static const float mapf(const float x, const float in_min, const float in_max, const float out_min, const float out_max)
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
    auto const START_MS = millis();

    static LGFX_Sprite co2Graph(&display);

    if (co2Graph.height() != h || co2Graph.width() != w)
    {
        co2Graph.setColorDepth(lgfx::rgb565_2Byte);
        co2Graph.setPsram(true);
        if (!co2Graph.createSprite(w, h))
        {
            Serial.println("could not resize sprite. halting and catching fire");
            return;
        }

        co2Graph.setTextSize(1);
        co2Graph.setTextWrap(false, false);
    }

    const auto GREEN_MAX_PPM = 500;
    const auto YELLOW_MAX_PPM = 650;
    const auto RED_MAX_PPM = 1000;

    const auto LOWEST_LEVEL_PPM = 400;
    const auto HIGHEST_LEVEL_PPM = 2000;

    const int GREEN_MAX_Y = mapf(GREEN_MAX_PPM, HIGHEST_LEVEL_PPM, LOWEST_LEVEL_PPM, 0, h);
    const int YELLOW_MAX_Y = mapf(YELLOW_MAX_PPM, HIGHEST_LEVEL_PPM, LOWEST_LEVEL_PPM, 0, h);
    const int RED_MAX_Y = mapf(RED_MAX_PPM, HIGHEST_LEVEL_PPM, LOWEST_LEVEL_PPM, 0, h);

    co2Graph.clear();

    // a single bar -1 pixel wide- with the required gradients
    static LGFX_Sprite bar(&co2Graph);

    constexpr const auto GREEN = bar.color565(0, 255, 0);
    constexpr const auto YELLOW = bar.color565(255, 255, 0);
    constexpr const auto RED = bar.color565(255, 0, 0);

    if (bar.height() != h)
    {
        bar.setColorDepth(lgfx::rgb565_2Byte);
        if (!bar.createSprite(1, h))
        {
            log_e("failed to (re)size sprite");
            return;
        };

        bar.drawLine(0, 0, 0, RED_MAX_Y, RED);
        bar.drawGradientLine(0, RED_MAX_Y, 0, YELLOW_MAX_Y, RED, YELLOW);
        bar.drawGradientLine(0, YELLOW_MAX_Y, 0, GREEN_MAX_Y, YELLOW, GREEN);
        bar.drawLine(0, GREEN_MAX_Y, 0, h, GREEN);
    }

    // now we use the bar to copy to the screen as a sprite or as separate pixels
    auto currentItem = 0;
    for (const auto &item : history)
    {
        const int BAR_HEIGHT = mapf(item.co2, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, 0, h);
        const auto xpos = co2Graph.width() - currentItem * (BAR_WIDTH + GAP_WIDTH);
        if (BAR_HEIGHT >= h)
        {
            auto cnt = 0;
            while (cnt < BAR_WIDTH)
                bar.pushSprite(xpos - BAR_WIDTH + cnt++, 0);
        }
        else
        {
            auto cnt = h - BAR_HEIGHT;
            while (cnt < h)
            {
                co2Graph.drawFastHLine(xpos - BAR_WIDTH, cnt, BAR_WIDTH, bar.readPixel(0, cnt));
                cnt++;
            }
        }
        currentItem++;
        if (xpos < 0)
            break;
    }

    // grid
    co2Graph.setTextDatum(CC_DATUM);
    co2Graph.setTextWrap(false, false);
    co2Graph.setTextColor(co2Graph.color565(192, 192, 192), 0);
    for (auto hgrid = 500; hgrid < HIGHEST_LEVEL_PPM; hgrid += 500)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, co2Graph.height(), 0);
        co2Graph.writeFastHLine(0, ypos, co2Graph.width(), co2Graph.color565(0, 0, 64));
        co2Graph.drawNumber(hgrid, co2Graph.width() >> 1, ypos, &DejaVu12);
    }

    co2Graph.pushSprite(x, y);

    log_v("total %lums", millis() - START_MS);
}

static void updateCo2Value(const int32_t w, const int32_t h, const int32_t x, const int32_t y, size_t newValue)
{
    static LGFX_Sprite co2Value(&display);

    if (co2Value.height() != h || co2Value.width() != w)
    {
        co2Value.setColorDepth(lgfx::palette_2bit);
        co2Value.setPsram(true);
        if (!co2Value.createSprite(w, h))
        {
            Serial.println("could not resize sprite. halting and catching fire");
            return;
        }
        co2Value.setTextSize(1);
        co2Value.setTextWrap(false, false);
    }

    co2Value.setPaletteColor(1, 0, 0, 255);
    co2Value.setPaletteColor(2, 31, 255, 31);
    co2Value.setPaletteColor(3, 180, 180, 180);
    co2Value.fillScreen(1);
    co2Value.setTextDatum(CC_DATUM);
    co2Value.setTextColor(2);
    co2Value.drawNumber(newValue, co2Value.width() >> 1, (co2Value.height() >> 1) + 4, &DejaVu40);

    const auto xMiddle = co2Value.width() >> 1;
    co2Value.drawString("CO²", xMiddle, 24, &DejaVu24Modded);
    co2Value.drawString("ppm", xMiddle, co2Value.height() - 24, &DejaVu24Modded);

    co2Value.pushSprite(x, y);
}

static void updateHumidityHistory(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    auto const START_MS = millis();

    static LGFX_Sprite humidityGraph(&display);

    if (humidityGraph.height() != h || humidityGraph.width() != w)
    {
        humidityGraph.setColorDepth(lgfx::rgb565_2Byte);
        humidityGraph.setPsram(true);
        if (!humidityGraph.createSprite(w, h))
        {
            Serial.println("could not resize sprite. halting and catching fire");
            return;
        }
        humidityGraph.setTextSize(1);
        humidityGraph.setTextWrap(false, false);
    }

    humidityGraph.clear();

    const auto RED_MAX_H = 70;
    const auto GREEN_MAX_H = 45;
    const auto WHITE_MAX_H = 25;

    const auto LOWEST_LEVEL_H = 15;
    const auto HIGHEST_LEVEL_H = 85;

    // a single bar -1 pixel wide- with the required gradients
    static LGFX_Sprite bar(&humidityGraph);

    if (bar.height() != h)
    {
        bar.setColorDepth(lgfx::rgb565_2Byte);
        if (!bar.createSprite(1, h))
        {
            log_e("failed to (re)size sprite");
            return;
        };

        const int RED_MAX_Y = mapf(RED_MAX_H, HIGHEST_LEVEL_H, LOWEST_LEVEL_H, 0, h);
        const int GREEN_MAX_Y = mapf(GREEN_MAX_H, HIGHEST_LEVEL_H, LOWEST_LEVEL_H, 0, h);
        const int WHITE_MAX_Y = mapf(WHITE_MAX_H, HIGHEST_LEVEL_H, LOWEST_LEVEL_H, 0, h);

        constexpr const auto WHITE = bar.color565(192, 192, 192);
        constexpr const auto GREEN = bar.color565(0, 255, 0);
        constexpr const auto RED = bar.color565(255, 0, 0);

        bar.drawLine(0, 0, 0, RED_MAX_Y, RED);
        bar.drawGradientLine(0, RED_MAX_Y, 0, GREEN_MAX_Y, RED, GREEN);
        bar.drawGradientLine(0, GREEN_MAX_Y, 0, WHITE_MAX_Y, GREEN, WHITE);
        bar.drawLine(0, WHITE_MAX_Y, 0, h, WHITE);
    }

    // now we use the bar to copy to the screen as a sprite or as separate pixels
    auto currentItem = 0;
    for (const auto &item : history)
    {
        const int BAR_HEIGHT = mapf(item.humidity, LOWEST_LEVEL_H, HIGHEST_LEVEL_H, 0, h);
        const auto xpos = humidityGraph.width() - currentItem * (BAR_WIDTH + GAP_WIDTH);
        if (BAR_HEIGHT >= h)
        {
            auto cnt = 0;
            while (cnt < BAR_WIDTH)
                bar.pushSprite(xpos - BAR_WIDTH + cnt++, 0);
        }
        else
        {
            auto cnt = h - BAR_HEIGHT;
            while (cnt < h)
            {
                humidityGraph.drawFastHLine(xpos - BAR_WIDTH, cnt, BAR_WIDTH, bar.readPixel(0, cnt));
                cnt++;
            }
        }
        currentItem++;
        if (xpos < 0)
            break;
    }

    // grid
    humidityGraph.setTextDatum(CC_DATUM);
    humidityGraph.setTextWrap(false, false);
    humidityGraph.setTextColor(humidityGraph.color565(192, 192, 192), 0);
    for (auto hgrid = 25; hgrid < HIGHEST_LEVEL_H; hgrid += 25)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_H, HIGHEST_LEVEL_H, humidityGraph.height(), 0);
        humidityGraph.writeFastHLine(0, ypos, humidityGraph.width(), humidityGraph.color565(0, 0, 64));
        humidityGraph.drawNumber(hgrid, humidityGraph.width() >> 1, ypos, &DejaVu12);
    }

    humidityGraph.pushSprite(x, y);

    log_v("total %lums", millis() - START_MS);
}

static void updateHumidityValue(const int32_t w, const int32_t h, const int32_t x, const int32_t y, size_t newValue)
{
    static LGFX_Sprite humidityValue(&display);

    if (humidityValue.height() != h || humidityValue.width() != w)
    {
        humidityValue.setColorDepth(lgfx::palette_2bit);
        humidityValue.setPsram(true);
        if (!humidityValue.createSprite(w, h))
        {
            Serial.println("could not resize sprite. halting and catching fire");
            return;
        }

        humidityValue.setTextSize(1);
        humidityValue.setTextWrap(false, false);
    }

    humidityValue.setPaletteColor(1, 0, 0, 255);
    humidityValue.setPaletteColor(2, 31, 255, 31);
    humidityValue.setPaletteColor(3, 180, 180, 180);
    humidityValue.fillScreen(1);

    humidityValue.setTextDatum(CC_DATUM);
    humidityValue.setTextColor(2);
    humidityValue.drawNumber(newValue, humidityValue.width() >> 1, (humidityValue.height() >> 1) + 4, &DejaVu40);

    const auto xMiddle = humidityValue.width() >> 1;
    humidityValue.drawString("RH", xMiddle, 24, &DejaVu24Modded);
    humidityValue.drawString("%", xMiddle, humidityValue.height() - 24, &DejaVu24Modded);

    humidityValue.pushSprite(x, y);
}

static void updateTempHistory(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    auto startMS = millis();

    static LGFX_Sprite tempGraph(&display);

    if (tempGraph.height() != h || tempGraph.width() != w)
    {
        tempGraph.setColorDepth(lgfx::rgb565_2Byte);
        tempGraph.setPsram(true);
        if (!tempGraph.createSprite(w, h))
        {
            Serial.println("could not resize sprite. halting and catching fire");
            return;
        }

        tempGraph.setTextSize(1);
        tempGraph.setTextWrap(false, false);
    }

    tempGraph.clear();

    const auto BLUE_MAX_T = 16;
    const auto GREEN_MAX_T = 20;
    const auto YELLOW_MAX_T = 22;
    const auto RED_MAX_T = 25;

    const auto LOWEST_LEVEL_T = 15;
    const auto HIGHEST_LEVEL_T = 28;

    const int BLUE_MAX_Y = mapf(BLUE_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);
    const int GREEN_MAX_Y = mapf(GREEN_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);
    const int YELLOW_MAX_Y = mapf(YELLOW_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);
    const int RED_MAX_Y = mapf(RED_MAX_T, HIGHEST_LEVEL_T, LOWEST_LEVEL_T, 0, h);

    // a single bar -1 pixel wide- with the required gradients
    static LGFX_Sprite bar(&tempGraph);

    constexpr const auto BLUE = bar.color565(95, 198, 224);
    constexpr const auto GREEN = bar.color565(0, 255, 0);
    constexpr const auto YELLOW = bar.color565(255, 255, 0);
    constexpr const auto RED = bar.color565(255, 0, 0);

    if (bar.height() != h)
    {
        bar.setColorDepth(lgfx::rgb565_2Byte);
        if (!bar.createSprite(1, h))
        {
            log_e("failed to (re)size sprite");
            return;
        };

        bar.drawLine(0, 0, 0, RED_MAX_Y, RED);
        bar.drawGradientLine(0, RED_MAX_Y, 0, YELLOW_MAX_Y, RED, YELLOW);
        bar.drawGradientLine(0, YELLOW_MAX_Y, 0, GREEN_MAX_Y, YELLOW, GREEN);
        bar.drawGradientLine(0, GREEN_MAX_Y, 0, BLUE_MAX_Y, GREEN, BLUE);
        bar.drawLine(0, BLUE_MAX_Y, 0, h, BLUE);
    }

    // now we use the bar to copy to the screen as a sprite or as separate pixels
    auto currentItem = 0;
    for (const auto &item : history)
    {
        const int BAR_HEIGHT = mapf(item.temp, LOWEST_LEVEL_T, HIGHEST_LEVEL_T, 0, h);
        const auto xpos = tempGraph.width() - currentItem * (BAR_WIDTH + GAP_WIDTH);
        if (BAR_HEIGHT >= h)
        {
            auto cnt = 0;
            while (cnt < BAR_WIDTH)
                bar.pushSprite(xpos - BAR_WIDTH + cnt++, 0);
        }
        else
        {
            auto cnt = h - BAR_HEIGHT;
            while (cnt < h)
            {
                tempGraph.drawFastHLine(xpos - BAR_WIDTH, cnt, BAR_WIDTH, bar.readPixel(0, cnt));
                cnt++;
            }
        }
        currentItem++;
        if (xpos < 0)
            break;
    }

    // grid
    tempGraph.setTextDatum(CC_DATUM);
    tempGraph.setTextWrap(false, false);
    tempGraph.setTextColor(tempGraph.color565(192, 192, 192), 0);
    for (auto hgrid = 16; hgrid < HIGHEST_LEVEL_T; hgrid += 4)
    {
        const auto ypos = mapf(hgrid, LOWEST_LEVEL_T, HIGHEST_LEVEL_T, tempGraph.height(), 0);
        tempGraph.writeFastHLine(0, ypos, tempGraph.width(), tempGraph.color565(0, 0, 64));
        tempGraph.drawNumber(hgrid, tempGraph.width() >> 1, ypos, &DejaVu12);
    }
    const auto pushTimeMS = millis();
    tempGraph.pushSprite(x, y);

    log_v("push time %lums", millis() - pushTimeMS);
    log_v("total %lums", millis() - startMS);
}

static void updateTempValue(const int32_t w, const int32_t h, const int32_t x, const int32_t y, float newValue)
{
    static LGFX_Sprite tempValue(&display);

    if (tempValue.height() != h || tempValue.width() != w)
    {
        tempValue.setColorDepth(lgfx::palette_2bit);
        tempValue.setPsram(true);
        if (!tempValue.createSprite(w, h))
        {
            Serial.println("could not resize sprite. halting and catching fire");
            return;
        }

        tempValue.setTextSize(1);
        tempValue.setTextWrap(false, false);
    }

    tempValue.setPaletteColor(1, 0, 0, 255);
    tempValue.setPaletteColor(2, 31, 255, 31);
    tempValue.setPaletteColor(3, 180, 180, 180);
    tempValue.fillScreen(1);

    tempValue.setTextDatum(CC_DATUM);
    tempValue.setTextColor(2);

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%.1f", newValue);

    char *integerStr;
    integerStr = strtok(buffer, ".");
    if (!integerStr)
        return;

    char *fractionStr;
    fractionStr = strtok(NULL, "\n");
    if (!fractionStr)
        return;

    // set the 40pt font
    tempValue.setFont(&DejaVu40);
    auto iWidth = tempValue.textWidth(integerStr);

    // set the 24pt font
    tempValue.setFont(&DejaVu24Modded);
    auto fWidth = tempValue.textWidth(fractionStr);

    auto xMiddle = tempValue.width() >> 1;
    auto yMiddle = tempValue.height() >> 1;

    auto bigNumberOffset = fWidth / 2;
    auto smallNumberOffset = bigNumberOffset - (iWidth + fWidth) / 2;

    tempValue.drawString(integerStr, xMiddle - bigNumberOffset, yMiddle, &DejaVu40);
    tempValue.drawString(fractionStr, xMiddle - smallNumberOffset, yMiddle - 6, &DejaVu24Modded);

    tempValue.drawString("T", xMiddle, 24, &DejaVu24Modded);
    tempValue.drawString("°C", xMiddle, tempValue.height() - 24, &DejaVu24Modded);

    tempValue.pushSprite(x, y);
}

void displayTask(void *parameter)
{
    display.setColorDepth(lgfx::rgb565_2Byte);
    display.init();
    display.clear(display.color565(0, 0, 255));
    display.setBrightness(130);
    display.setTextWrap(false, false);
    display.setTextScroll(false);

    while (1)
    {
        const auto DEFAULT_DELAY_MS = 5;
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
            if (msg.type != displayMessage::SYSTEM_MESSAGE)
                delay(DEFAULT_DELAY_MS);
        }
        /*
                int32_t x, y;
                if (display.getTouch(&x, &y))
                    display.fillRect(x - 2, y - 2, 5, 5, TFT_BLUE);
        */
        struct tm timeinfo = {0};
        static struct tm prevTime = {0};
        if (getLocalTime(&timeinfo, 0) && prevTime.tm_sec != timeinfo.tm_sec)
        {
            // TODO: first draw the time 88:88:88 in a very lightcolored font with the background overwrite
            // then draw the current time over that - requires a sprite
            char timestr[16];
            strftime(timestr, sizeof(timestr), "%X", &timeinfo); // https://cplusplus.com/reference/ctime/strftime/
            display.setTextColor(display.color565(253, 32, 32), TFT_BLUE);
            display.setTextDatum(CC_DATUM);
            display.setTextSize(1.6);
            display.drawString(timestr, display.width() >> 1, 400, &Font7);
            prevTime = timeinfo;
        }
        yield();
    }
}