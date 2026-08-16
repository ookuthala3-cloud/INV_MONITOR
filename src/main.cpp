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

  Serial.println("INV_MONITOR Page 1 UI started");
}

void loop() {
  static uint32_t lastFrame = 0;

  // ~20 FPS
  if (millis() - lastFrame >= 50) {
    lastFrame = millis();

    updateDemoTargets();
    updateAnimations();
    drawPage1();
  }
}
