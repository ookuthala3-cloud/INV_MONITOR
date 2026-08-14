#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "pins.h"

// Custom SPI bus
SPIClass displaySPI(FSPI);

// ST7789 display
Adafruit_ST7789 tft =
    Adafruit_ST7789(&displaySPI, TFT_CS, TFT_DC, TFT_RST);


void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println(" ESP32-S3 INVERTER MONITOR");
    Serial.println(" ST7789 Display Test");
    Serial.println("================================");


    // -----------------------------
    // Backlight
    // -----------------------------

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);


    // -----------------------------
    // SPI
    // SCLK, MISO, MOSI, SS
    // -----------------------------

    displaySPI.begin(
        TFT_SCLK,
        -1,
        TFT_MOSI,
        TFT_CS
    );


    // -----------------------------
    // ST7789 240x240
    // -----------------------------

    tft.init(240, 240);

    tft.setRotation(0);

    tft.fillScreen(ST77XX_BLACK);


    // -----------------------------
    // Header
    // -----------------------------

    tft.setTextWrap(false);

    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);

    tft.setCursor(26, 20);
    tft.println("INV MONITOR");


    // -----------------------------
    // Divider
    // -----------------------------

    tft.drawFastHLine(
        10,
        48,
        220,
        ST77XX_DARKGREY
    );


    // -----------------------------
    // Display test
    // -----------------------------

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(25, 75);
    tft.println("ST7789 240x240");


    tft.setTextColor(ST77XX_GREEN);

    tft.setCursor(55, 110);
    tft.println("DISPLAY");


    tft.setCursor(75, 140);
    tft.println("OK");


    // -----------------------------
    // RGB test
    // -----------------------------

    tft.fillRect(
        35,
        185,
        50,
        25,
        ST77XX_RED
    );

    tft.fillRect(
        95,
        185,
        50,
        25,
        ST77XX_GREEN
    );

    tft.fillRect(
        155,
        185,
        50,
        25,
        ST77XX_BLUE
    );


    Serial.println("Display initialized.");
}


void loop()
{
    // Display test only
}
