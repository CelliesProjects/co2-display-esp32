#include "displayTask.hpp"

static float mapf(const float x, const float in_min, const float in_max, const float out_min, const float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void showSystemMessage(char *str)
{
    static LGFX_Sprite sysMess(&display);

    if (sysMess.width() == 0 || sysMess.height() == 0)
    {
        sysMess.setColorDepth(lgfx::palette_2bit);
        if (!sysMess.createSprite(display.width(), GRAPH_HEIGHT))
        {
            log_e("could not create sprite");
            return;
        }
        sysMess.setPaletteColor(1, 200, 200, 200);
        sysMess.setTextColor(1);
    }
    sysMess.clear();

    auto cnt = 0;
    char *pch = strtok(str, "\n");
    while (pch)
    {
        sysMess.drawCenterString(pch, display.width() >> 1, cnt++ * DejaVu24Modded.yAdvance, &DejaVu24Modded);
        pch = strtok(NULL, "\n");
    }

    sysMess.pushSprite(0, GRAPH_HEIGHT + 5);
}

void showTimeLines(LGFX_Sprite &sprite, bool showString = false)
{
    constexpr uint32_t dist30 = 30 * (BAR_WIDTH + GAP_WIDTH);
    constexpr uint32_t dist60 = dist30 * 2;
    constexpr uint16_t lineColor = lgfx::color565(164, 164, 164);

    sprite.drawFastVLine(sprite.width() - dist30, 0, sprite.height(), lineColor);
    sprite.drawFastVLine(sprite.width() - dist60, 0, sprite.height(), lineColor);

    if (!showString)
        return;

    sprite.setTextDatum(TC_DATUM);
    sprite.drawString("-30 min", sprite.width() - dist30, 5, &DejaVu12);
    sprite.drawString("-60 min", sprite.width() - dist60, 5, &DejaVu12);
}

static void updateCo2History(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    [[maybe_unused]] auto const START_MS = millis();

    static LGFX_Sprite co2Graph(&display);

    if (co2Graph.height() != h || co2Graph.width() != w)
    {
        co2Graph.setColorDepth(lgfx::rgb565_2Byte);
        co2Graph.setPsram(true);
        co2Graph.setBaseColor(GRAPH_BACKGROUND);
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

    // first we construct a single bar -1 pixel wide- with the desired gradients
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

    // draw horizontal lines and the corresponding co2 values
    co2Graph.setTextDatum(CC_DATUM);
    co2Graph.setTextWrap(false, false);
    co2Graph.setTextColor(lgfx::color565(192, 192, 192), GRAPH_BACKGROUND);
    for (auto gridLineHeight = 500; gridLineHeight < HIGHEST_LEVEL_PPM; gridLineHeight += 500)
    {
        const auto ypos = mapf(gridLineHeight, LOWEST_LEVEL_PPM, HIGHEST_LEVEL_PPM, co2Graph.height(), 0);
        co2Graph.writeFastHLine(0, ypos, co2Graph.width(), lgfx::color565(0, 0, 64));
        co2Graph.drawNumber(gridLineHeight, co2Graph.width() >> 1, ypos, &DejaVu12);
    }
    showTimeLines(co2Graph, true);

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
    co2Value.drawNumber(newValue, co2Value.width() >> 1, (co2Value.height() >> 1) + 4, &DejaVu40Modded);

    const auto xMiddle = co2Value.width() >> 1;
    co2Value.drawString("CO²", xMiddle, 24, &DejaVu24Modded);
    co2Value.drawString("ppm", xMiddle, co2Value.height() - 24, &DejaVu24Modded);

    co2Value.pushSprite(x, y);
}

static void updateHumidityHistory(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    [[maybe_unused]] auto const START_MS = millis();

    static LGFX_Sprite humidityGraph(&display);

    if (humidityGraph.height() != h || humidityGraph.width() != w)
    {
        humidityGraph.setColorDepth(lgfx::rgb565_2Byte);
        humidityGraph.setBaseColor(GRAPH_BACKGROUND);
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
    const auto GREEN_MAX_H_HIGH = 60;
    const auto GREEN_MAX_H_LOW = 40;
    const auto WHITE_MAX_H = 30;

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
        const int GREEN_MAX_Y_HIGH = mapf(GREEN_MAX_H_HIGH, HIGHEST_LEVEL_H, LOWEST_LEVEL_H, 0, h);
        const int GREEN_MAX_Y_LOW = mapf(GREEN_MAX_H_LOW, HIGHEST_LEVEL_H, LOWEST_LEVEL_H, 0, h);
        const int WHITE_MAX_Y = mapf(WHITE_MAX_H, HIGHEST_LEVEL_H, LOWEST_LEVEL_H, 0, h);

        constexpr const auto WHITE = bar.color565(192, 192, 192);
        constexpr const auto GREEN = bar.color565(0, 255, 0);
        constexpr const auto RED = bar.color565(255, 0, 0);

        bar.drawLine(0, 0, 0, RED_MAX_Y, RED);
        bar.drawGradientLine(0, RED_MAX_Y, 0, GREEN_MAX_Y_HIGH, RED, GREEN);
        bar.drawLine(0, GREEN_MAX_Y_HIGH, 0, GREEN_MAX_Y_LOW, GREEN);
        bar.drawGradientLine(0, GREEN_MAX_Y_LOW, 0, WHITE_MAX_Y, GREEN, WHITE);
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
    humidityGraph.setTextColor(humidityGraph.color565(192, 192, 192), GRAPH_BACKGROUND);
    for (auto gridLineHeight = 25; gridLineHeight < HIGHEST_LEVEL_H; gridLineHeight += 25)
    {
        const auto ypos = mapf(gridLineHeight, LOWEST_LEVEL_H, HIGHEST_LEVEL_H, humidityGraph.height(), 0);
        humidityGraph.writeFastHLine(0, ypos, humidityGraph.width(), humidityGraph.color565(0, 0, 64));
        humidityGraph.drawNumber(gridLineHeight, humidityGraph.width() >> 1, ypos, &DejaVu12);
    }

    showTimeLines(humidityGraph);

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
    humidityValue.drawNumber(newValue, humidityValue.width() >> 1, (humidityValue.height() >> 1) + 4, &DejaVu40Modded);

    const auto xMiddle = humidityValue.width() >> 1;
    humidityValue.drawString("RH", xMiddle, 24, &DejaVu24Modded);
    humidityValue.drawString("%", xMiddle, humidityValue.height() - 24, &DejaVu24Modded);

    humidityValue.pushSprite(x, y);
}

static void updateTempHistory(const int32_t w, const int32_t h, const int32_t x, const int32_t y)
{
    [[maybe_unused]] auto const START_MS = millis();

    static LGFX_Sprite tempGraph(&display);

    if (tempGraph.height() != h || tempGraph.width() != w)
    {
        tempGraph.setColorDepth(lgfx::rgb565_2Byte);
        tempGraph.setPsram(true);
        tempGraph.setBaseColor(GRAPH_BACKGROUND);
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
    tempGraph.setTextColor(tempGraph.color565(192, 192, 192), GRAPH_BACKGROUND);
    for (auto gridLineHeight = 16; gridLineHeight < HIGHEST_LEVEL_T; gridLineHeight += 4)
    {
        const auto ypos = mapf(gridLineHeight, LOWEST_LEVEL_T, HIGHEST_LEVEL_T, tempGraph.height(), 0);
        tempGraph.writeFastHLine(0, ypos, tempGraph.width(), tempGraph.color565(0, 0, 64));
        tempGraph.drawNumber(gridLineHeight, tempGraph.width() >> 1, ypos, &DejaVu12);
    }

    showTimeLines(tempGraph);

    [[maybe_unused]] const auto pushTimeMS = millis();
    tempGraph.pushSprite(x, y);

    log_v("push time %lums", millis() - pushTimeMS);
    log_v("total %lums", millis() - START_MS);
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
    tempValue.setFont(&DejaVu40Modded);
    auto iWidth = tempValue.textWidth(integerStr);

    // set the 24pt font
    tempValue.setFont(&DejaVu24Modded);
    auto fWidth = tempValue.textWidth(fractionStr);

    auto xMiddle = tempValue.width() >> 1;
    auto yMiddle = tempValue.height() >> 1;

    auto bigNumberOffset = fWidth / 2;
    auto smallNumberOffset = bigNumberOffset - (iWidth + fWidth) / 2;

    tempValue.setTextColor(2);
    tempValue.drawString(integerStr, xMiddle - bigNumberOffset, yMiddle, &DejaVu40Modded);
    tempValue.drawString(fractionStr, xMiddle - smallNumberOffset, yMiddle - 4, &DejaVu24Modded);

    tempValue.drawString("T", xMiddle, 24, &DejaVu24Modded);
    tempValue.drawString("°C", xMiddle, tempValue.height() - 24, &DejaVu24Modded);

    tempValue.pushSprite(x, y);
}

struct iconData
{
    const uint8_t *start;
    const uint8_t *end;
};

static iconData selectIcon(const char *weather)
{
    if (strcmp(weather, "clear-day") == 0)
        return {clear_day_png_start, clear_day_png_end};
    if (strcmp(weather, "clear-night") == 0)
        return {clear_night_png_start, clear_night_png_end};
    if (strcmp(weather, "cloudy") == 0)
        return {cloudy_png_start, cloudy_png_end};
    if (strcmp(weather, "fog") == 0)
        return {fog_png_start, fog_png_end};
    if (strcmp(weather, "hail") == 0)
        return {hail_png_start, hail_png_end};
    if (strcmp(weather, "partly-cloudy-day") == 0)
        return {partly_cloudy_day_png_start, partly_cloudy_day_png_end};
    if (strcmp(weather, "partly-cloudy-night") == 0)
        return {partly_cloudy_night_png_start, partly_cloudy_night_png_end};
    if (strcmp(weather, "rain") == 0)
        return {rain_png_start, rain_png_end};
    if (strcmp(weather, "rain-snow") == 0)
        return {rain_snow_png_start, rain_snow_png_end};
    if (strcmp(weather, "rain-snow-showers-day") == 0)
        return {rain_snow_showers_day_png_start, rain_snow_showers_day_png_end};
    if (strcmp(weather, "rain-snow-showers-night") == 0)
        return {rain_snow_showers_night_png_start, rain_snow_showers_night_png_end};
    if (strcmp(weather, "showers-day") == 0)
        return {showers_day_png_start, showers_day_png_end};
    if (strcmp(weather, "showers-night") == 0)
        return {showers_night_png_start, showers_night_png_end};
    if (strcmp(weather, "sleet") == 0)
        return {sleet_png_start, sleet_png_end};
    if (strcmp(weather, "snow") == 0)
        return {snow_png_start, snow_png_end};
    if (strcmp(weather, "snow-showers-day") == 0)
        return {snow_showers_day_png_start, snow_showers_day_png_end};
    if (strcmp(weather, "snow-showers-night") == 0)
        return {snow_showers_night_png_start, snow_showers_night_png_end};
    if (strcmp(weather, "thunder") == 0)
        return {thunder_png_start, thunder_png_end};
    if (strcmp(weather, "thunder-rain") == 0)
        return {thunder_rain_png_start, thunder_rain_png_end};
    if (strcmp(weather, "thunder-showers-day") == 0)
        return {thunder_showers_day_png_start, thunder_showers_day_png_end};
    if (strcmp(weather, "thunder-showers-night") == 0)
        return {thunder_showers_night_png_start, thunder_showers_night_png_end};
    if (strcmp(weather, "wind") == 0)
        return {wind_png_start, wind_png_end};

    return {nullptr, nullptr}; // Fallback if no match found
}

static void updateWeatherForecast(const int32_t w, const int32_t h, const int32_t x, const int32_t y, const char *icon, const float temp)
{
    static LGFX_Sprite weather(&display);

    if (weather.height() != h || weather.width() != w)
    {
        weather.setColorDepth(lgfx::rgb565_2Byte);
        weather.setBaseColor(WEATHER_BACKGROUND);
        weather.setPsram(true);
        if (!weather.createSprite(w, h))
        {
            log_e("could not resize sprite. halting and catching fire");
            return;
        }
        weather.setFont(&DejaVu24Modded);
        weather.setTextSize(1);
        weather.setTextWrap(false, false);
    }

    weather.clear();

    weather.setTextColor(0);
    weather.drawCenterString("weather forecast", weather.width() >> 1, 1, &DejaVu12);
    weather.drawCenterString("visualcrossing.com", weather.width() >> 1, weather.height() - 14, &DejaVu12);

    const iconData png = selectIcon(icon);
    if (png.start && png.end && !weather.drawPng(png.start, png.end - png.start, 30, 15))
        weather.drawString("PNG ERROR!", 30, 15, &DejaVu12);

    weather.setTextColor(weather.color565(20, 20, 20));

    if (!isnan(temp))
    {
        char buff[10];
        snprintf(buff, sizeof(buff), "%.0f°", temp);
        weather.setTextDatum(CC_DATUM);
        weather.drawString(buff, weather.width() - 55, (weather.height() >> 1) + 4, &DejaVu40Modded);
    }

    weather.pushSprite(x, y);
}

void showForecast(const char *icon, const float temp)
{
    updateWeatherForecast(display.width() - 285, 96, 285, GRAPH_HEIGHT * 3 + 15, icon, temp);
}

static void handleMessage(displayMessage &msg)
{
    switch (msg.type)
    {
    case displayMessage::SYSTEM_MESSAGE:
    {
        showSystemMessage(msg.str);
        break;
    }

    case displayMessage::CO2_HISTORY:
    {
        updateCo2History(GRAPH_WIDTH, GRAPH_HEIGHT, 0, 0);
        break;
    }

    case displayMessage::CO2_LEVEL:
    {
        updateCo2Value(VALUE_WIDTH, GRAPH_HEIGHT, GRAPH_WIDTH, 0, msg.sizeVal);
        break;
    }

    case displayMessage::HUMIDITY_HISTORY:
    {
        updateHumidityHistory(GRAPH_WIDTH, GRAPH_HEIGHT, 0, GRAPH_HEIGHT + 5);
        break;
    }

    case displayMessage::HUMIDITY:
    {
        updateHumidityValue(VALUE_WIDTH, GRAPH_HEIGHT, GRAPH_WIDTH, GRAPH_HEIGHT + 5, msg.sizeVal);
        break;
    }

    case displayMessage::TEMPERATURE_HISTORY:
    {
        updateTempHistory(GRAPH_WIDTH, GRAPH_HEIGHT, 0, GRAPH_HEIGHT * 2 + 10);
        break;
    }

    case displayMessage::TEMPERATURE:
    {
        updateTempValue(VALUE_WIDTH, GRAPH_HEIGHT, GRAPH_WIDTH, GRAPH_HEIGHT * 2 + 10, msg.floatVal);
        break;
    }

    default:
        log_w("unhandled tft msg type");
    }
}

static void updateClock(const struct tm &timeinfo)
{
    constexpr const auto font = &Font7;
    static LGFX_Sprite clock(&display);
    clock.setPsram(true);
    clock.setBaseColor(CLOCK_BACKGROUND);
    clock.setTextDatum(CC_DATUM);
    clock.setTextSize(2);

    char timestr[16] = "88:88";
    const auto width = clock.textWidth(timestr, font);
    const auto height = clock.fontHeight(font);

    if (clock.width() != width || clock.height() != height)
    {
        if (!clock.createSprite(width, height))
        {
            log_e("could not create sprite");
            return;
        }
        clock.clear();
    }

    clock.setTextColor(clock.color565(204, 122, 0));
    const auto xMiddle = width >> 1;
    const auto yMiddle = height >> 1;
    clock.drawString(timestr, xMiddle, yMiddle, font);

    if (timeinfo.tm_year != 0)
    {
        strftime(timestr, sizeof(timestr), "%R", &timeinfo);
        clock.setTextColor(display.color565(20, 12, 6));
        clock.drawString(timestr, xMiddle, yMiddle, font);
    }

    clock.pushSprite(0, GRAPH_HEIGHT * 3 + 15);
}

bool TFTtouched(int32_t &x, int32_t &y)
{
    return display.getTouch(&x, &y) ? true : false;
}

void takeScreenshot()
{
    ScreenShot sShot;
    String error;
    log_i("Saving screenshot to SD");
    bool success = sShot.saveBMP("/screenshot.bmp", display, SD, error);
    log_i("Screenshot: %s", success ? "saved" : error.c_str());
}

void handleTouch()
{
    static bool touchActive = false;
    static bool screenshotTaken = false;
    static uint32_t touchStartMs = 0;

    int32_t tx, ty;
    const bool touching = display.getTouch(&tx, &ty);

    constexpr uint32_t HOLD_TIME_MS = 1200;

    if (!touching)
    {
        touchActive = false;
        screenshotTaken = false;
        return;
    }

    if (tx < 440 || ty > 40)
    {
        touchActive = false;
        screenshotTaken = false;
        return;
    }

    if (!touchActive)
    {
        touchActive = true;
        screenshotTaken = false;
        touchStartMs = millis();
    }
    else if (!screenshotTaken && (millis() - touchStartMs >= HOLD_TIME_MS))
    {
        takeScreenshot();
        screenshotTaken = true;
    }
}

void displayTask(void *parameter)
{
    display.setColorDepth(lgfx::rgb565_2Byte);
    display.init();
    display.clear(BACKGROUND_COLOR);
    display.setBrightness(100);
    display.setTextWrap(false, false);
    display.setTextScroll(false);

    const struct tm timeinfo = {};
    updateClock(timeinfo);
    updateWeatherForecast(display.width() - 285, 96, 285, GRAPH_HEIGHT * 3 + 15, "", NAN);

    constexpr const auto TICK_RATE_HZ = 50;
    constexpr const TickType_t ticksToWait = pdTICKS_TO_MS(1000 / TICK_RATE_HZ);
    static TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, ticksToWait);

        static struct displayMessage msg;
        if (xQueueReceive(displayQueue, &msg, 0) == pdTRUE)
            handleMessage(msg);
        else
        {
            struct tm timeinfo = {};
            static struct tm prevTime = {};
            if (getLocalTime(&timeinfo, 0) && prevTime.tm_sec != timeinfo.tm_sec)
            {
                updateClock(timeinfo);
                prevTime = timeinfo;
            }
        }
        handleTouch();
    }
}
