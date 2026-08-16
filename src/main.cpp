#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite ui = TFT_eSprite(&tft);

static const int W = 240;
static const int H = 240;

// ---------- UI colors ----------
uint16_t C_BG;
uint16_t C_PANEL;
uint16_t C_GRID;
uint16_t C_WHITE;
uint16_t C_MUTED;
uint16_t C_GREEN;
uint16_t C_BLUE;
uint16_t C_ORANGE;
uint16_t C_YELLOW;
uint16_t C_RED;

// ---------- animated display values ----------
float vDisp = 0.0f;
float aDisp = 0.0f;
float wDisp = 0.0f;
float hzDisp = 0.0f;
float pfDisp = 0.0f;
float battDisp = 0.0f;
float battVDisp = 0.0f;
float tempDisp = 0.0f;

// ---------- target values (demo for UI test) ----------
float vTarget = 230.4f;
float aTarget = 2.42f;
float wTarget = 557.0f;
float hzTarget = 50.0f;
float pfTarget = 0.98f;
float battTarget = 82.0f;
float battVTarget = 12.6f;
float tempTarget = 38.0f;

// ---------- Page 2 demo values ----------
float battMinV = 12.1f;
float battMaxV = 13.8f;
float battAvgV = 12.5f;
float dischargeDisp = 0.0f;
float dischargeTarget = 5.6f;

// ---------- Page 4 demo environment values ----------
float ambientTempDisp = 0.0f;
float ambientTempTarget = 29.5f;
float pressureDisp = 0.0f;
float pressureTarget = 1008.0f;

// ---------- Page manager ----------
uint8_t currentPage = 0;
uint32_t lastPageSwitch = 0;
const uint32_t PAGE_INTERVAL = 15000UL;  // 15 seconds
float wavePhase = 0.0f;

// ---------- Page 5 power graph ----------
static const int POWER_SAMPLES = 60;
float powerHistory[POWER_SAMPLES];
uint32_t lastPowerSample = 0;
float powerMaxSeen = 0.0f;
float powerAvg = 0.0f;

static inline float smoothTo(float current, float target, float k = 0.14f) {
  return current + (target - current) * k;
}

static inline float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void drawArcSegment(
  TFT_eSprite &s,
  int cx, int cy,
  int radius,
  int thickness,
  float startDeg,
  float endDeg,
  uint16_t color
) {
  // Smooth rounded thick arc, compatible with TFT_eSprite.
  const float DEG = 0.01745329252f;
  const int dotR = max(1, thickness / 2);

  for (float a = startDeg; a <= endDeg; a += 1.25f) {
    int x = cx + (int)roundf(cosf(a * DEG) * radius);
    int y = cy + (int)roundf(sinf(a * DEG) * radius);
    s.fillCircle(x, y, dotR, color);
  }

  int sx = cx + (int)roundf(cosf(startDeg * DEG) * radius);
  int sy = cy + (int)roundf(sinf(startDeg * DEG) * radius);
  int ex = cx + (int)roundf(cosf(endDeg * DEG) * radius);
  int ey = cy + (int)roundf(sinf(endDeg * DEG) * radius);

  s.fillCircle(sx, sy, dotR, color);
  s.fillCircle(ex, ey, dotR, color);
}

void drawGauge(
  TFT_eSprite &s,
  int cx, int cy,
  int r,
  float value,
  float minV,
  float maxV,
  const char *label,
  const char *unit,
  uint16_t activeColor
) {
  const float startA = 150.0f;
  const float endA   = 390.0f;
  const float span   = endA - startA;

  float p = (value - minV) / (maxV - minV);
  p = clampf(p, 0.0f, 1.0f);

  // background arc
  drawArcSegment(s, cx, cy, r, 7, startA, endA, C_GRID);

  // active arc
  float activeEnd = startA + span * p;
  if (activeEnd > startA + 1.0f) {
    drawArcSegment(s, cx, cy, r, 7, startA, activeEnd, activeColor);
  }

  // center value
  s.setTextDatum(MC_DATUM);
  s.setTextColor(C_WHITE, C_BG);
  s.setTextFont(2);

  char buf[24];

  if (strcmp(unit, "W") == 0) {
    snprintf(buf, sizeof(buf), "%.0f", value);
  } else if (strcmp(unit, "A") == 0) {
    snprintf(buf, sizeof(buf), "%.2f", value);
  } else {
    snprintf(buf, sizeof(buf), "%.1f", value);
  }

  s.drawString(buf, cx, cy - 2);
  s.setTextColor(C_MUTED, C_BG);
  s.setTextFont(1);
  s.drawString(unit, cx, cy + 11);

  s.setTextColor(activeColor, C_BG);
  s.setTextFont(1);
  s.drawString(label, cx, cy + r + 7);
}

void drawBattery(int x, int y, int w, int h, float percent) {
  percent = clampf(percent, 0, 100);

  ui.drawRoundRect(x, y, w, h, 3, C_GREEN);
  ui.fillRect(x + w, y + h / 3, 3, h / 3, C_GREEN);

  int innerW = w - 4;
  int fillW = (int)(innerW * (percent / 100.0f));

  ui.fillRect(x + 2, y + 2, innerW, h - 4, C_PANEL);
  ui.fillRect(x + 2, y + 2, fillW, h - 4, C_GREEN);
}

void drawPage1() {
  ui.fillSprite(C_BG);

  // outer card
  ui.drawRoundRect(3, 3, 234, 234, 10, C_BLUE);

  // ---------- Header ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextColor(C_WHITE, C_BG);
  ui.setTextFont(2);
  ui.drawString("MAIN / INVERTER", 10, 9);

  ui.setTextDatum(TR_DATUM);
  ui.setTextColor(C_MUTED, C_BG);
  ui.setTextFont(1);
  ui.drawString("WiFi", 229, 11);

  ui.drawFastHLine(8, 27, 224, C_GRID);

  // ---------- Title ----------
  ui.setTextDatum(MC_DATUM);
  ui.setTextColor(C_YELLOW, C_BG);
  ui.setTextFont(2);
  ui.drawString("AC OUTPUT", 120, 39);

  // ---------- Three gauges ----------
  drawGauge(ui, 43, 82, 28, vDisp, 180.0f, 260.0f, "VOLT", "V", C_GREEN);
  drawGauge(ui, 120, 82, 28, aDisp, 0.0f, 20.0f, "CURRENT", "A", C_BLUE);
  drawGauge(ui, 197, 82, 28, wDisp, 0.0f, 1200.0f, "POWER", "W", C_ORANGE);

  ui.drawFastHLine(8, 121, 224, C_GRID);

  // ---------- Frequency + PF ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("FREQ", 16, 130);
  ui.drawString("PF", 135, 130);

  char buf[32];
  ui.setTextFont(2);

  ui.setTextColor(C_BLUE, C_BG);
  snprintf(buf, sizeof(buf), "%.1f Hz", hzDisp);
  ui.drawString(buf, 16, 142);

  ui.setTextColor(C_WHITE, C_BG);
  snprintf(buf, sizeof(buf), "%.2f", pfDisp);
  ui.drawString(buf, 135, 142);

  ui.drawFastVLine(118, 128, 27, C_GRID);
  ui.drawFastHLine(8, 162, 224, C_GRID);

  // ---------- Battery ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("BATTERY", 16, 171);

  drawBattery(16, 185, 42, 18, battDisp);

  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);

  snprintf(buf, sizeof(buf), "%.1fV", battVDisp);
  ui.drawString(buf, 68, 180);

  snprintf(buf, sizeof(buf), "%.0f%%", battDisp);
  ui.drawString(buf, 68, 196);

  // ---------- System status ----------
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("SYSTEM", 136, 171);

  ui.setTextFont(2);
  ui.setTextColor(C_GREEN, C_BG);
  ui.drawString("NORMAL", 136, 186);

  ui.fillCircle(216, 190, 9, C_GREEN);
  ui.setTextDatum(MC_DATUM);
  ui.setTextColor(C_BG, C_GREEN);
  ui.setTextFont(2);
  ui.drawString("OK", 216, 190);

  ui.drawFastHLine(8, 211, 224, C_GRID);

  // ---------- Temperature ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("INVERTER TEMP", 16, 219);

  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);
  snprintf(buf, sizeof(buf), "%.1f C", tempDisp);
  ui.drawString(buf, 225, 216);

  ui.pushSprite(0, 0);
}

void drawInfoCard(int x, int y, int w, int h,
                  const char *label, float value, uint16_t color) {
  ui.fillRoundRect(x, y, w, h, 5, C_PANEL);
  ui.drawRoundRect(x, y, w, h, 5, C_GRID);

  ui.setTextDatum(MC_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_PANEL);
  ui.drawString(label, x + w / 2, y + 8);

  char buf[18];
  snprintf(buf, sizeof(buf), "%.1f V", value);

  ui.setTextFont(2);
  ui.setTextColor(color, C_PANEL);
  ui.drawString(buf, x + w / 2, y + 24);
}

void drawBatteryGaugeLarge(float percent, float voltage) {
  percent = clampf(percent, 0.0f, 100.0f);

  const int cx = 120;
  const int cy = 79;
  const int r = 45;
  const float startA = 150.0f;
  const float endA = 390.0f;
  const float span = endA - startA;

  drawArcSegment(ui, cx, cy, r, 9, startA, endA, C_GRID);

  float activeEnd = startA + span * (percent / 100.0f);
  if (activeEnd > startA + 1.0f) {
    drawArcSegment(ui, cx, cy, r, 9, startA, activeEnd, C_GREEN);
  }

  char buf[20];

  ui.setTextDatum(MC_DATUM);
  ui.setTextFont(4);
  ui.setTextColor(C_GREEN, C_BG);
  snprintf(buf, sizeof(buf), "%.0f%%", percent);
  ui.drawString(buf, cx, cy - 4);

  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);
  snprintf(buf, sizeof(buf), "%.1f V", voltage);
  ui.drawString(buf, cx, cy + 22);
}

void drawSegmentBar(int x, int y, int w, int h, float value, float maxValue) {
  const int segments = 10;
  const int gap = 2;
  int segW = (w - gap * (segments - 1)) / segments;

  float p = clampf(value / maxValue, 0.0f, 1.0f);
  int active = (int)roundf(p * segments);

  for (int i = 0; i < segments; i++) {
    int sx = x + i * (segW + gap);

    uint16_t color = C_GRID;

    if (i < active) {
      if (i < 6) color = C_GREEN;
      else if (i < 8) color = C_YELLOW;
      else color = C_RED;
    }

    ui.fillRoundRect(sx, y, segW, h, 2, color);
  }
}

void drawPage2() {
  ui.fillSprite(C_BG);

  // outer card
  ui.drawRoundRect(3, 3, 234, 234, 10, C_GREEN);

  // ---------- Header ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextColor(C_WHITE, C_BG);
  ui.setTextFont(2);
  ui.drawString("BATTERY & DC", 10, 9);

  ui.setTextDatum(TR_DATUM);
  ui.setTextColor(C_MUTED, C_BG);
  ui.setTextFont(1);
  ui.drawString("2 / 2", 229, 11);

  ui.drawFastHLine(8, 27, 224, C_GRID);

  // ---------- Main battery gauge ----------
  drawBatteryGaugeLarge(battDisp, battVDisp);

  // ---------- Min / Max / Avg cards ----------
  drawInfoCard(8,   127, 70, 39, "MIN", battMinV, C_BLUE);
  drawInfoCard(85,  127, 70, 39, "MAX", battMaxV, C_ORANGE);
  drawInfoCard(162, 127, 70, 39, "AVG", battAvgV, C_GREEN);

  // ---------- Discharge current ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("DISCHARGE CURRENT", 12, 174);

  char buf[20];
  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(2);
  ui.setTextColor(C_BLUE, C_BG);
  snprintf(buf, sizeof(buf), "%.1f A", dischargeDisp);
  ui.drawString(buf, 228, 170);

  drawSegmentBar(12, 192, 216, 10, dischargeDisp, 20.0f);

  // ---------- Status ----------
  ui.drawFastHLine(8, 211, 224, C_GRID);

  drawBattery(12, 218, 28, 12, battDisp);

  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("STATUS", 49, 218);

  ui.setTextFont(2);
  ui.setTextColor(C_GREEN, C_BG);
  ui.drawString("NORMAL", 94, 215);

  ui.fillCircle(218, 223, 7, C_GREEN);
  ui.setTextDatum(MC_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_BG, C_GREEN);
  ui.drawString("OK", 218, 223);

  ui.pushSprite(0, 0);
}

void drawMiniWaveform(int x, int y, int w, int h) {
  ui.drawRoundRect(x, y, w, h, 4, C_GRID);

  // center line
  int midY = y + h / 2;
  ui.drawFastHLine(x + 3, midY, w - 6, C_GRID);

  // faint vertical grid
  for (int gx = x + 18; gx < x + w; gx += 18) {
    ui.drawFastVLine(gx, y + 3, h - 6, C_PANEL);
  }

  // sine waveform
  int prevX = x + 3;
  int prevY = midY;
  for (int px = 1; px < w - 6; px++) {
    float a = (px / (float)(w - 6)) * 6.2831853f * 1.7f + wavePhase;
    int py = midY + (int)(sinf(a) * (h * 0.30f));
    int xx = x + 3 + px;
    ui.drawLine(prevX, prevY, xx, py, C_BLUE);
    prevX = xx;
    prevY = py;
  }
}

void drawACValue(int x, int y, const char *label,
                 const char *value, uint16_t color) {
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString(label, x, y);

  ui.setTextFont(2);
  ui.setTextColor(color, C_BG);
  ui.drawString(value, x, y + 11);
}

void drawPage3() {
  ui.fillSprite(C_BG);

  // outer card
  ui.drawRoundRect(3, 3, 234, 234, 10, C_BLUE);

  // ---------- Header ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);
  ui.drawString("AC DETAILS", 10, 9);

  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("3 / 3", 229, 11);

  ui.drawFastHLine(8, 27, 224, C_GRID);

  char buf[24];

  // ---------- Left column ----------
  snprintf(buf, sizeof(buf), "%.1f V", vDisp);
  drawACValue(12, 38, "VOLTAGE (RMS)", buf, C_BLUE);

  snprintf(buf, sizeof(buf), "%.2f A", aDisp);
  drawACValue(12, 72, "CURRENT (RMS)", buf, C_GREEN);

  snprintf(buf, sizeof(buf), "%.0f W", wDisp);
  drawACValue(12, 106, "POWER", buf, C_ORANGE);

  snprintf(buf, sizeof(buf), "%.2f", pfDisp);
  drawACValue(12, 140, "POWER FACTOR", buf, C_YELLOW);

  // vertical divider
  ui.drawFastVLine(112, 35, 137, C_GRID);

  // ---------- Right column ----------
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("FREQUENCY", 123, 38);

  ui.setTextFont(2);
  ui.setTextColor(C_YELLOW, C_BG);
  snprintf(buf, sizeof(buf), "%.1f Hz", hzDisp);
  ui.drawString(buf, 123, 49);

  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("AC WAVEFORM", 123, 74);

  drawMiniWaveform(122, 88, 105, 65);

  // ---------- Load type ----------
  ui.drawFastHLine(8, 174, 224, C_GRID);

  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("LOAD TYPE", 12, 184);

  ui.setTextFont(2);
  ui.setTextColor(C_GREEN, C_BG);
  ui.drawString("RESISTIVE", 12, 197);

  // ---------- Output status ----------
  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("OUTPUT", 226, 184);

  ui.setTextFont(2);
  ui.setTextColor(C_GREEN, C_BG);
  ui.drawString("STABLE", 226, 197);

  ui.drawFastHLine(8, 217, 224, C_GRID);

  ui.setTextDatum(MC_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("LIVE AC MONITOR", 120, 226);

  ui.pushSprite(0, 0);
}

void drawTempBar(int x, int y, int w, int h, float tempC) {
  // 0..80 C display range
  float p = clampf(tempC / 80.0f, 0.0f, 1.0f);

  ui.drawRoundRect(x, y, w, h, 4, C_GRID);

  int innerW = w - 4;
  int fillW = (int)(innerW * p);

  // Draw vivid sections according to current temperature.
  for (int i = 0; i < fillW; i++) {
    float q = i / (float)innerW;
    uint16_t c = C_GREEN;
    if (q >= 0.60f && q < 0.80f) c = C_YELLOW;
    else if (q >= 0.80f) c = C_RED;
    ui.drawFastVLine(x + 2 + i, y + 2, h - 4, c);
  }
}

void drawPage4() {
  ui.fillSprite(C_BG);

  ui.drawRoundRect(3, 3, 234, 234, 10, C_ORANGE);

  // Header
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);
  ui.drawString("TEMPERATURE", 10, 9);

  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("4 / 4", 229, 11);

  ui.drawFastHLine(8, 27, 224, C_GRID);

  // Main inverter temperature gauge
  const int cx = 120;
  const int cy = 79;
  const int r = 43;
  const float startA = 150.0f;
  const float endA = 390.0f;

  float tp = clampf(tempDisp / 80.0f, 0.0f, 1.0f);
  uint16_t tempColor = C_GREEN;
  if (tempDisp >= 60.0f) tempColor = C_RED;
  else if (tempDisp >= 45.0f) tempColor = C_YELLOW;

  drawArcSegment(ui, cx, cy, r, 9, startA, endA, C_GRID);
  drawArcSegment(ui, cx, cy, r, 9, startA,
                 startA + (endA - startA) * tp, tempColor);

  char buf[28];

  ui.setTextDatum(MC_DATUM);
  ui.setTextFont(4);
  ui.setTextColor(tempColor, C_BG);
  snprintf(buf, sizeof(buf), "%.1f", tempDisp);
  ui.drawString(buf, cx, cy - 5);

  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);
  ui.drawString("C", cx, cy + 20);

  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("INVERTER TEMP", cx, 122);

  ui.drawFastHLine(8, 134, 224, C_GRID);

  // Ambient temperature
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("AMBIENT", 13, 143);

  ui.setTextFont(2);
  ui.setTextColor(C_BLUE, C_BG);
  snprintf(buf, sizeof(buf), "%.1f C", ambientTempDisp);
  ui.drawString(buf, 13, 155);

  // Pressure
  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("PRESSURE", 227, 143);

  ui.setTextFont(2);
  ui.setTextColor(C_YELLOW, C_BG);
  snprintf(buf, sizeof(buf), "%.0f hPa", pressureDisp);
  ui.drawString(buf, 227, 155);

  // Temperature range bar
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("TEMPERATURE RANGE", 13, 181);

  drawTempBar(13, 194, 214, 10, tempDisp);

  // Bottom status
  ui.drawFastHLine(8, 212, 224, C_GRID);

  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("STATUS", 13, 220);

  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(2);

  if (tempDisp >= 60.0f) {
    ui.setTextColor(C_RED, C_BG);
    ui.drawString("HOT", 226, 217);
  } else if (tempDisp >= 45.0f) {
    ui.setTextColor(C_YELLOW, C_BG);
    ui.drawString("WARM", 226, 217);
  } else {
    ui.setTextColor(C_GREEN, C_BG);
    ui.drawString("NORMAL", 226, 217);
  }

  ui.pushSprite(0, 0);
}

void updatePowerHistory() {
  uint32_t now = millis();
  if (now - lastPowerSample < 1000UL) return;
  lastPowerSample = now;

  for (int i = 0; i < POWER_SAMPLES - 1; i++) {
    powerHistory[i] = powerHistory[i + 1];
  }
  powerHistory[POWER_SAMPLES - 1] = wDisp;

  float sum = 0.0f;
  powerMaxSeen = 0.0f;
  for (int i = 0; i < POWER_SAMPLES; i++) {
    sum += powerHistory[i];
    if (powerHistory[i] > powerMaxSeen) powerMaxSeen = powerHistory[i];
  }
  powerAvg = sum / POWER_SAMPLES;
}

void drawPowerGraph(int x, int y, int w, int h) {
  const float graphMax = 1000.0f;

  ui.drawRoundRect(x, y, w, h, 4, C_GRID);

  // Horizontal grid: 0, 250, 500, 750, 1000 W
  for (int i = 0; i <= 4; i++) {
    int gy = y + 3 + ((h - 6) * i) / 4;
    ui.drawFastHLine(x + 3, gy, w - 6, C_GRID);
  }

  // Vertical time grid
  for (int i = 1; i < 4; i++) {
    int gx = x + (w * i) / 4;
    for (int yy = y + 4; yy < y + h - 3; yy += 4) {
      ui.drawPixel(gx, yy, C_GRID);
    }
  }

  int prevX = x + 3;
  float first = clampf(powerHistory[0], 0.0f, graphMax);
  int prevY = y + h - 4 - (int)((first / graphMax) * (h - 8));

  for (int i = 1; i < POWER_SAMPLES; i++) {
    int px = x + 3 + (i * (w - 7)) / (POWER_SAMPLES - 1);
    float val = clampf(powerHistory[i], 0.0f, graphMax);
    int py = y + h - 4 - (int)((val / graphMax) * (h - 8));

    // subtle vertical fill
    for (int fy = py + 3; fy < y + h - 4; fy += 5) {
      ui.drawPixel(px, fy, C_PANEL);
    }

    // vivid orange trace, doubled for visibility
    ui.drawLine(prevX, prevY, px, py, C_ORANGE);
    ui.drawLine(prevX, prevY + 1, px, py + 1, C_ORANGE);

    prevX = px;
    prevY = py;
  }
}

void drawPage5() {
  ui.fillSprite(C_BG);
  ui.drawRoundRect(3, 3, 234, 234, 10, C_ORANGE);

  // Header
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);
  ui.drawString("POWER GRAPH", 10, 9);

  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("5 / 5", 229, 11);
  ui.drawFastHLine(8, 27, 224, C_GRID);

  char buf[24];

  // Current power
  ui.setTextDatum(TL_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("LIVE POWER", 12, 35);

  ui.setTextFont(4);
  ui.setTextColor(C_ORANGE, C_BG);
  snprintf(buf, sizeof(buf), "%.0f", wDisp);
  ui.drawString(buf, 12, 48);

  int numberWidth = ui.textWidth(buf);
  ui.setTextFont(2);
  ui.setTextColor(C_WHITE, C_BG);
  ui.drawString("W", 18 + numberWidth, 59);

  // Max / Average
  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("MAX", 226, 36);
  ui.setTextFont(2);
  ui.setTextColor(C_RED, C_BG);
  snprintf(buf, sizeof(buf), "%.0f W", powerMaxSeen);
  ui.drawString(buf, 226, 47);

  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("AVG", 226, 64);
  ui.setTextFont(2);
  ui.setTextColor(C_GREEN, C_BG);
  snprintf(buf, sizeof(buf), "%.0f W", powerAvg);
  ui.drawString(buf, 226, 75);

  // Graph
  drawPowerGraph(29, 94, 198, 91);

  // Y-axis labels
  ui.setTextDatum(TR_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("1000", 26, 94);
  ui.drawString("750", 26, 115);
  ui.drawString("500", 26, 137);
  ui.drawString("250", 26, 159);
  ui.drawString("0", 26, 178);

  // Time labels
  ui.setTextDatum(MC_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("-60s", 32, 194);
  ui.drawString("-45s", 79, 194);
  ui.drawString("-30s", 128, 194);
  ui.drawString("-15s", 177, 194);
  ui.drawString("0s", 220, 194);

  ui.drawFastHLine(8, 207, 224, C_GRID);

  // Range selector look
  ui.setTextDatum(MC_DATUM);
  ui.setTextFont(1);
  ui.setTextColor(C_ORANGE, C_BG);
  ui.drawString("LIVE", 34, 221);

  ui.setTextColor(C_MUTED, C_BG);
  ui.drawString("1H", 82, 221);
  ui.drawString("6H", 126, 221);
  ui.drawString("12H", 171, 221);
  ui.drawString("24H", 216, 221);

  ui.drawFastHLine(17, 231, 34, C_ORANGE);

  ui.pushSprite(0, 0);
}

void updatePageManager() {
  uint32_t now = millis();

  if (now - lastPageSwitch >= PAGE_INTERVAL) {
    lastPageSwitch = now;
    currentPage = (currentPage + 1) % 5;
  }
}

void updateDemoTargets() {
  // Small movement so gauge smoothing can be checked.
  float t = millis() * 0.001f;

  vTarget = 230.4f + sinf(t * 0.55f) * 2.3f;
  aTarget = 2.42f + sinf(t * 0.72f) * 0.35f;
  wTarget = 557.0f + sinf(t * 0.61f) * 55.0f;
  hzTarget = 50.0f + sinf(t * 0.25f) * 0.08f;
  pfTarget = 0.98f + sinf(t * 0.20f) * 0.01f;
  battTarget = 82.0f;
  battVTarget = 12.6f;
  tempTarget = 38.0f + sinf(t * 0.15f) * 0.7f;

  // Page 2 demo movement
  dischargeTarget = 5.6f + sinf(t * 0.50f) * 0.8f;

  ambientTempTarget = 29.5f + sinf(t * 0.18f) * 0.6f;
  pressureTarget = 1008.0f + sinf(t * 0.10f) * 2.5f;

  wavePhase += 0.12f;
  if (wavePhase > 6.2831853f) wavePhase -= 6.2831853f;
}

void updateAnimations() {
  vDisp = smoothTo(vDisp, vTarget);
  aDisp = smoothTo(aDisp, aTarget);
  wDisp = smoothTo(wDisp, wTarget);
  hzDisp = smoothTo(hzDisp, hzTarget);
  pfDisp = smoothTo(pfDisp, pfTarget);
  battDisp = smoothTo(battDisp, battTarget, 0.10f);
  battVDisp = smoothTo(battVDisp, battVTarget, 0.10f);
  tempDisp = smoothTo(tempDisp, tempTarget, 0.10f);
  dischargeDisp = smoothTo(dischargeDisp, dischargeTarget, 0.12f);
  ambientTempDisp = smoothTo(ambientTempDisp, ambientTempTarget, 0.10f);
  pressureDisp = smoothTo(pressureDisp, pressureTarget, 0.08f);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // RGB565 palette
  C_BG     = TFT_BLACK;
  C_PANEL  = tft.color565(12, 12, 12);
  C_GRID   = tft.color565(55, 55, 55);
  C_WHITE  = tft.color565(255, 255, 255);
  C_MUTED  = tft.color565(150, 150, 150);
  C_GREEN  = tft.color565(0, 255, 0);
  C_BLUE   = tft.color565(0, 170, 255);
  C_ORANGE = tft.color565(255, 110, 0);
  C_YELLOW = tft.color565(255, 255, 0);
  C_RED    = tft.color565(255, 0, 0);

  ui.setColorDepth(16);

  if (psramFound()) {
    ui.setAttribute(PSRAM_ENABLE, true);
    Serial.println("PSRAM: OK");
  } else {
    Serial.println("PSRAM: not found");
  }

  if (!ui.createSprite(W, H)) {
    Serial.println("ERROR: sprite allocation failed");
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Sprite alloc failed", 20, 110, 2);
    while (true) delay(1000);
  }

  ui.fillSprite(C_BG);

  // Start near target values so boot animation is pleasant.
  vDisp = 220.0f;
  aDisp = 1.0f;
  wDisp = 300.0f;
  hzDisp = 49.5f;
  pfDisp = 0.90f;
  battDisp = 60.0f;
  battVDisp = 11.8f;
  tempDisp = 32.0f;
  dischargeDisp = 3.0f;
  ambientTempDisp = 27.0f;
  pressureDisp = 1000.0f;

  for (int i = 0; i < POWER_SAMPLES; i++) {
    powerHistory[i] = 500.0f + sinf(i * 0.28f) * 75.0f;
  }
  lastPowerSample = millis();
  powerMaxSeen = 575.0f;
  powerAvg = 500.0f;

  currentPage = 0;
  lastPageSwitch = millis();

  Serial.println("INV_MONITOR Page 1 to Page 5 UI started");
  Serial.println("Auto rotate: 15 seconds");
}

void loop() {
  static uint32_t lastFrame = 0;

  updatePageManager();
  updatePowerHistory();

  // ~20 FPS
  if (millis() - lastFrame >= 50) {
    lastFrame = millis();

    updateDemoTargets();
    updateAnimations();

    if (currentPage == 0) {
      drawPage1();
    } else if (currentPage == 1) {
      drawPage2();
    } else if (currentPage == 2) {
      drawPage3();
    } else if (currentPage == 3) {
      drawPage4();
    } else {
      drawPage5();
    }
  }
}
