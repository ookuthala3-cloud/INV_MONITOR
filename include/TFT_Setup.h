#ifndef TFT_SETUP_H
#define TFT_SETUP_H
#define USE_HSPI_PORT

// ==========================================
// ST7789 240x240
// ==========================================
#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// Red/Blue correction for our LCD
#define TFT_RGB_ORDER TFT_BGR

// ==========================================
// Bruce working pin mapping
// ==========================================
#define TFT_MOSI 6
#define TFT_SCLK 5
#define TFT_MISO 41

#define TFT_CS   16
#define TFT_DC   7
#define TFT_RST  15

#define TFT_BL   4
#define TFT_BACKLIGHT_ON HIGH

// ==========================================
// Fonts
// ==========================================
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

// ==========================================
// SPI
// ==========================================
#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  16000000

#endif
