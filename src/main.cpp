#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <FS.h>

#include <ScioSense_ENS160.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SHT31.h>


#include <Arduino_GFX_Library.h> // https://github.com/moononournation/Arduino_GFX/
#include <TouchLib.h>            // https://github.com/mmMicky/TouchLib

// touch stuff
#define PIN_TOUCH_SCL 45
#define PIN_TOUCH_SDA 19
#define PIN_TOUCH_INT 1
#define PIN_TOUCH_RES 2

TouchLib touch(Wire, PIN_TOUCH_SDA, PIN_TOUCH_SCL, GT911_SLAVE_ADDRESS1);

// sensor stuff
#define PIN_SENSOR_SCL 40
#define PIN_SENSOR_SDA 2

//  because Wire is only internally connected to the touchpad
//  the ens160 sensor has to be on Wire1 and routed
//  through the exposed GPIOs on the back of the device
TwoWire I2Csensor = TwoWire(1);

ScioSense_ENS160 ens160(&I2Csensor, ENS160_I2CADDR_1);
Adafruit_AHTX0 aht;

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

void printAirQuality()
{
    // Give values to Air Quality Sensor.
    ens160.set_envdata210(tempC, humidity);
    ens160.measure(false);
    ens160.measureRaw(false);
    gfx->setCursor(10, 10);
    gfx->setTextColor(0xffff, 0x0);
    gfx->setTextSize(6 /* x scale */, 8 /* y scale */, 1 /* pixel_margin */);
    gfx->printf("TVOC: %06i", ens160.getTVOC());
    gfx->setCursor(10, 100);
    gfx->printf("CO2: %06i", ens160.geteCO2());
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

void updateTemp()
{
    sensors_event_t humidity1, temp; // Tim had to change to humidity1
    aht.getEvent(&humidity1, &temp); // populate temp and humidity objects with fresh data
    tempC = (temp.temperature);
    tempF = (temp.temperature) * 1.8 + 32;
    humidity = (humidity1.relative_humidity);
    /*
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" degrees C");
    Serial.print("Temperature: ");
    Serial.print(tempF);
    Serial.println(" degrees F");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("% rH");
    */
}

void showAirQuality()
{
    if (ens160.available())
    {
        // Give values to Air Quality Sensor.
        ens160.set_envdata210(tempC, humidity);
        ens160.measure(false);
        ens160.measureRaw(false);
        /*
                Serial.print("AQI: ");
                Serial.print(ens160.getAQI());
                Serial.print("\t");

                Serial.print("TVOC: ");
                Serial.print(ens160.getTVOC());
                Serial.print("ppb\t");

                Serial.print("eCO2: ");
                Serial.print(ens160.geteCO2());
                Serial.println("ppm\t");
                */
    }
}

void setup()
{
    Serial.begin(115200);

    // mount sd card
    SPI.begin(48, 41, 47);
    if (!SD.begin(42))
        Serial.println("SD card mount failed");
    else
        Serial.println("SD card mounted");

    // display
    ledcSetup(0, 1220, SOC_LEDC_TIMER_BIT_WIDE_NUM);
    ledcAttachPin(GFX_BL, 0);
    ledcWrite(0, (1ul << SOC_LEDC_TIMER_BIT_WIDE_NUM - 2));

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
    touch.init();

    // set second I2C interface pins
    I2Csensor.setPins(PIN_SENSOR_SDA, PIN_SENSOR_SCL);

    // aht sensor
    bool aresult = aht.begin(&I2Csensor);
    Serial.printf("aht sensor is %s\n", aresult ? "found" : "absent");
    delay(100);
    // co2 sensor
    bool mresult = ens160.begin(false);
    Serial.printf("environment sensor is %s\n", mresult ? "found" : "absent");
    ens160.setMode(ENS160_OPMODE_STD);
}

void loop()
{
    static auto updateTimer = time(NULL);

    drawCurrentTouchpoints();
    if (time(NULL) != updateTimer)
    {
        updateTemp();
        printAirQuality();
        updateTimer = time(NULL);
    }
    delay(5);
}