#include "displayTask.hpp"

void printHelloWorld()
{
    gfx->setCursor(random(gfx->width()), random(gfx->height()));
    gfx->setTextColor(random(0xffff), random(0xffff));
    gfx->setTextSize(random(6) /* x scale */, random(6) /* y scale */, random(2) /* pixel_margin */);
    gfx->println("Hello World!");
}

void drawCurrentTouchpoints()
{
    if (touch.read())
    {
        uint8_t n = touch.getPointNum();
        /// Serial.printf("getPointNum: %d  ", n);
        for (uint8_t i = 0; i < n; i++)
        {
            TP_Point t = touch.getPoint(i);
            // Serial.printf("[%d] point x: %d  point y: %d \r\n", i, t.x, t.y);
            gfx->drawCircle(t.x, t.y, 10, BLACK);
        }
    }
}

void updateCO2()
{
}

void updateTempHumidity()
{
}

void displayTask(void *parameter)
{
    // display backlight
    ledcSetup(0, 1220, SOC_LEDC_TIMER_BIT_WIDE_NUM);
    ledcAttachPin(GFX_BL, 0);
    ledcWrite(0, (1ul << SOC_LEDC_TIMER_BIT_WIDE_NUM - 2));

    // display
    gfx->begin(16000000);
    gfx->setRotation(0);
    gfx->fillScreen(RED);
    gfx->print("Hello world!");

    // touch panel
    pinMode(PIN_TOUCH_RES, OUTPUT);
    digitalWrite(PIN_TOUCH_RES, 0);
    delay(200);
    digitalWrite(PIN_TOUCH_RES, 1);
    delay(200);
    if (!touch.init())
        Serial.println("touch not started");
    else
        Serial.println("touch started");

    while (1)
    {
        drawCurrentTouchpoints();
        delay(4);
    }
}