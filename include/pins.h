#ifndef PINS_H
#define PINS_H

// =====================================================
// ESP32-S3 INVERTER MONITOR
// Hardware Pin Configuration
// =====================================================


// -----------------------------------------------------
// ST7789 240x240 TFT DISPLAY
// SPI
// -----------------------------------------------------

#define TFT_SCLK        12
#define TFT_MOSI        11
#define TFT_CS          10
#define TFT_DC           9
#define TFT_RST          8
#define TFT_BL           7


// -----------------------------------------------------
// BMP180
// I2C
// -----------------------------------------------------

#define I2C_SDA          5
#define I2C_SCL          6


// -----------------------------------------------------
// ANALOG SENSORS
// -----------------------------------------------------

// ACS712 - AC Current
#define ACS712_PIN       4

// ZMPT101B - AC Voltage
#define ZMPT101B_PIN     3

// 0-25V Voltage Sensor - Battery/DC Voltage
#define DC_VOLTAGE_PIN   2


// -----------------------------------------------------
// BUZZER
// -----------------------------------------------------

#define BUZZER_PIN      21


// -----------------------------------------------------
// BUILT-IN MICRO SD
// 1-bit SD interface
// -----------------------------------------------------

#define SD_DATA_PIN     40
#define SD_CLK_PIN      39
#define SD_CMD_PIN      38


// -----------------------------------------------------
// RESERVED PINS
// DO NOT USE
// -----------------------------------------------------

// Built-in PSRAM
#define PSRAM_PIN_1     35
#define PSRAM_PIN_2     36
#define PSRAM_PIN_3     37


// -----------------------------------------------------
// USB
// Avoid using these while USB is required
// -----------------------------------------------------

#define USB_DM_PIN      20
#define USB_DP_PIN      19


#endif
