#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=== INV_MONITOR TFT_eSPI TEST ===");

    tft.init();
    tft.setRotation(0);

    Serial.println("TFT initialized");

    // BLACK
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    tft.drawString(
        "INV_MONITOR",
        120,
        70,
        4
    );

    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(
        "ESP32-S3",
        120,
        110,
        2
    );

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(
        "ST7789 240x240",
        120,
        140,
        2
    );

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(
        "TFT_eSPI OK",
        120,
        175,
        2
    );

    // Border
    tft.drawRoundRect(
        5,
        5,
        230,
        230,
        12,
        TFT_BLUE
    );

    Serial.println("Display test completed");
}

void loop()
{
}
