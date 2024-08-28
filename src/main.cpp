#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <FS.h>
#include <driver/uart.h>

#include <Adafruit_SHT31.h>
#include <s8_uart.h>

#include <Arduino_GFX_Library.h> // https://github.com/moononournation/Arduino_GFX/

// touch stuff
#define TOUCH_MODULES_GT911
#include <TouchLib.h> // https://github.com/mmMicky/TouchLib

#define PIN_TOUCH_SCL 45
#define PIN_TOUCH_SDA 19
#define PIN_TOUCH_INT 1
#define PIN_TOUCH_RES 2

TouchLib touch(Wire, PIN_TOUCH_SDA, PIN_TOUCH_SCL, GT911_SLAVE_ADDRESS1);

// sensor stuff
#define PIN_SENSOR_SCL 40
#define PIN_SENSOR_SDA 2

//  because Wire is only internally connected to the touchpad
//  the sht31 sensor has to be on Wire1 and routed
//  through the exposed GPIOs on the back of the device
TwoWire I2Csensor = TwoWire(1);

Adafruit_SHT31 sht31(&I2Csensor);

// co2 sensor is a serial device 9600-8-n-1
#define PIN_S8_RXD 43
#define PIN_S8_TXD 44

HardwareSerial S8_serial(2);
S8_UART *sensor_S8;

// display stuff
#define GFX_BL 38

Arduino_DataBus *bus = new Arduino_SWSPI(
    GFX_NOT_DEFINED /* DC */, 39 /* CS */,
    48 /* SCK */, 47 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    18 /* DE */, 17 /* VSYNC */, 16 /* HSYNC */, 21 /* PCLK */,
    11 /* R0 */, 12 /* R1 */, 13 /* R2 */, 14 /* R3 */, 0 /* R4 */,
    8 /* G0 */, 20 /* G1 */, 3 /* G2 */, 46 /* G3 */, 9 /* G4 */, 10 /* G5 */,
    4 /* B0 */, 5 /* B1 */, 6 /* B2 */, 7 /* B3 */, 15 /* B4 */,
    1 /* hsync_polarity */, 10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
    1 /* vsync_polarity */, 10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    480 /* width */, 480 /* height */, rgbpanel, 1 /* rotation */, true /* auto_flush */,
    bus, GFX_NOT_DEFINED /* RST */, st7701_type9_init_operations, sizeof(st7701_type9_init_operations));

int tempC;    // To store the temperature in C
int tempF;    // temp in F
int humidity; // To store the humidity

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

void setup()
{
    Serial.begin(115200);
    // Serial.setDebugOutput(true);

    // mount sd card
    SPI.begin(48, 41, 47);
    if (!SD.begin(42))
        Serial.println("SD card mount failed");
    else
        Serial.println("SD card mounted");

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
    Serial.println("Starting touch panel");

    pinMode(PIN_TOUCH_RES, OUTPUT);
    digitalWrite(PIN_TOUCH_RES, 0);
    delay(200);
    digitalWrite(PIN_TOUCH_RES, 1);
    delay(200);
    touch.init();

    Serial.println("Setting Second I2C pins");

    // set second I2C interface pins
    I2Csensor.setPins(PIN_SENSOR_SDA, PIN_SENSOR_SCL);

    Serial.println("Starting sht31");

    bool result = sht31.begin(SHT31_DEFAULT_ADDR);
    Serial.printf("sht31 sensor is %s\n", result ? "found" : "absent");

    Serial.flush();

    // start SenseAir S8 sensor on TXD 43 and RXD 44
    S8_serial.setPins(PIN_S8_RXD, PIN_S8_TXD);
    S8_serial.begin(S8_BAUDRATE);
    sensor_S8 = new S8_UART(S8_serial);

    Serial.println("we have a sensor");

    S8_sensor sensor;
    sensor.co2 = sensor_S8->get_co2();
    Serial.print("CO2 value = ");
    Serial.print(sensor.co2);
    Serial.println(" ppm");

    Serial.println("setup done");
}

void updateCO2()
{
    S8_sensor sensor;
    sensor.co2 = sensor_S8->get_co2();
    gfx->setCursor(50, 100);
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(5 /* x scale */, 5 /* y scale */, 1 /* pixel_margin */);
    gfx->printf("CO2 % 5i ppm", sensor.co2);
}

void updateTempHumidity()
{
    const float t = sht31.readTemperature();
    const float h = sht31.readHumidity();

    if (!isnan(t))
    {
        gfx->setCursor(50, 150);
        gfx->setTextColor(WHITE, BLACK);
        gfx->setTextSize(5 /* x scale */, 5 /* y scale */, 1 /* pixel_margin */);
        gfx->printf("T % 3.1f C", t);
    }
    if (!isnan(h))
    {
        gfx->setCursor(50, 200);
        gfx->setTextColor(WHITE, BLACK);
        gfx->setTextSize(5 /* x scale */, 5 /* y scale */, 1 /* pixel_margin */);
        gfx->printf("H % 3.1f%%%", h);
    }
}

void loop()
{
    static auto tempTimer = 0;
    static auto co2Timer = 0;

    drawCurrentTouchpoints();
    if (time(NULL) != tempTimer)
    {
        updateTempHumidity();
        tempTimer = time(NULL);
    }
    if (time(NULL) != co2Timer)
    {
        updateCO2();
        co2Timer = time(NULL);
    }
    delay(5);
}