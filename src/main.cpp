#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

constexpr int SCREEN_WIDTH = 240;
constexpr int SCREEN_HEIGHT = 320;
constexpr uint8_t TOUCH_CS_PIN = 33;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite(&tft);
XPT2046_Touchscreen touch(TOUCH_CS_PIN);

struct Vec3 {
  float x;
  float y;
  float z;
};

struct Vec2 {
  int16_t x;
  int16_t y;
};

const Vec3 CUBE[8] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}
};

const uint8_t EDGE[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

float angleX = -0.6f;
float angleY = 0.8f;
bool wasTouched = false;
int16_t lastX = 0;
int16_t lastY = 0;
constexpr int TOUCH_MOVE_THRESHOLD = 5;
constexpr float ROTATE_SENSITIVITY = 0.0035f;

Vec2 project(const Vec3 &p) {
  float sx = sinf(angleX);
  float cx = cosf(angleX);
  float sy = sinf(angleY);
  float cy = cosf(angleY);

  float x1 = p.x;
  float y1 = p.y * cx - p.z * sx;
  float z1 = p.y * sx + p.z * cx;

  float x2 = x1 * cy + z1 * sy;
  float z2 = -x1 * sy + z1 * cy;

  float scale = 78.0f;
  float depth = 4.0f;
  float perspective = depth / (depth + z2);

  Vec2 out;
  out.x = static_cast<int16_t>(SCREEN_WIDTH / 2 + x2 * scale * perspective);
  out.y = static_cast<int16_t>(SCREEN_HEIGHT / 2 + y1 * scale * perspective + 10);
  return out;
}

void drawCubeFrame() {
  Vec2 pt[8];
  for (int i = 0; i < 8; ++i) {
    pt[i] = project(CUBE[i]);
  }

  sprite.fillSprite(TFT_BLACK);
  sprite.fillRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 12, TFT_NAVY);
  sprite.setTextDatum(TC_DATUM);
  sprite.setTextColor(TFT_CYAN, TFT_NAVY);
  sprite.drawString("CUBE 3D", SCREEN_WIDTH / 2, 8, 2);

  for (int i = 0; i < 12; ++i) {
    uint8_t a = EDGE[i][0];
    uint8_t b = EDGE[i][1];
    uint16_t color = (i < 4 || i >= 8) ? TFT_GREEN : TFT_YELLOW;
    sprite.drawLine(pt[a].x, pt[a].y, pt[b].x, pt[b].y, color);
  }

  sprite.setTextDatum(BC_DATUM);
  sprite.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  sprite.drawString("KEO DE XOAY", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 8, 2);
  sprite.pushSprite(0, 0);
}

void readTouchRotation() {
  if (!touch.touched()) {
    wasTouched = false;
    return;
  }

  TS_Point p = touch.getPoint();
  int16_t x = p.x;
  int16_t y = p.y;

  if (wasTouched) {
    int16_t dx = x - lastX;
    int16_t dy = y - lastY;

    if (abs(dx) > TOUCH_MOVE_THRESHOLD || abs(dy) > TOUCH_MOVE_THRESHOLD) {
      angleY += dx * ROTATE_SENSITIVITY;
      angleX += dy * ROTATE_SENSITIVITY;
      angleX = constrain(angleX, -1.4f, 1.4f);
      drawCubeFrame();
    }
  }

  lastX = x;
  lastY = y;
  wasTouched = true;
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_RED);
  delay(200);

  sprite.setColorDepth(8);
  if (!sprite.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
    Serial.println("Sprite allocation failed");
    tft.fillScreen(TFT_RED);
    while (true) {
      delay(1000);
    }
  }

  touch.begin();
  touch.setRotation(1);

  drawCubeFrame();
  Serial.println("Cube ready");
}

void loop() {
  readTouchRotation();
  delay(20);
}

