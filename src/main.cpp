#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

constexpr int SCREEN_WIDTH = 320;
constexpr int SCREEN_HEIGHT = 240;
constexpr uint8_t TOUCH_IRQ_PIN = 36;
constexpr uint8_t TOUCH_MOSI_PIN = 32;
constexpr uint8_t TOUCH_MISO_PIN = 39;
constexpr uint8_t TOUCH_SCLK_PIN = 25;
constexpr uint8_t TOUCH_CS_PIN = 33;
constexpr float AUTO_ROTATION_SPEED = 1.0f;
constexpr int16_t TOUCH_RAW_MIN_X = 200;
constexpr int16_t TOUCH_RAW_MAX_X = 3700;
constexpr int16_t TOUCH_RAW_MIN_Y = 240;
constexpr int16_t TOUCH_RAW_MAX_Y = 3800;
constexpr int16_t BUTTON_WIDTH = 88;
constexpr int16_t BUTTON_GAP = 8;
constexpr int16_t BUTTON_Y = 202;
constexpr int16_t BUTTON_HEIGHT = 30;
constexpr float TOUCH_ROTATE_SENSITIVITY = 0.012f;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite(&tft);
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(TOUCH_CS_PIN, TOUCH_IRQ_PIN);

enum class ScreenMode {
  Cube,
  TouchTest
};

ScreenMode screenMode = ScreenMode::Cube;

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
uint32_t lastRotationUpdate = 0;
bool touchWasDown = false;
Vec2 lastTouch = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
constexpr int16_t CUBE_BUTTON_X = 8;
constexpr int16_t TOUCH_BUTTON_X = CUBE_BUTTON_X + BUTTON_WIDTH + BUTTON_GAP;

struct MagicParticle {
  Vec2 position;
  Vec2 velocity;
  uint16_t color;
  uint8_t life;
};

constexpr uint8_t MAGIC_PARTICLE_COUNT = 28;
MagicParticle magicParticles[MAGIC_PARTICLE_COUNT];

bool isInsideButton(const Vec2 &point, int16_t x, int16_t width) {
  return point.x >= x && point.x < x + width &&
         point.y >= BUTTON_Y && point.y < BUTTON_Y + BUTTON_HEIGHT;
}

Vec2 readTouchPoint(const TS_Point &point) {
  Vec2 result;
  result.x = map(point.x, TOUCH_RAW_MIN_X, TOUCH_RAW_MAX_X, 1, SCREEN_WIDTH);
  result.y = map(point.y, TOUCH_RAW_MIN_Y, TOUCH_RAW_MAX_Y, 1, SCREEN_HEIGHT);
  result.x = constrain(result.x, 0, SCREEN_WIDTH - 1);
  result.y = constrain(result.y, 0, SCREEN_HEIGHT - 1);
  return result;
}

void drawNavigation() {
  uint16_t cubeColor = screenMode == ScreenMode::Cube ? TFT_CYAN : TFT_DARKGREY;
  uint16_t touchColor =
      screenMode == ScreenMode::TouchTest ? TFT_MAGENTA : TFT_DARKGREY;

  sprite.fillRoundRect(CUBE_BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 6,
                       cubeColor);
  sprite.fillRoundRect(TOUCH_BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 6,
                       touchColor);
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE, cubeColor);
  sprite.drawString("CUBE", CUBE_BUTTON_X + BUTTON_WIDTH / 2,
                    BUTTON_Y + BUTTON_HEIGHT / 2, 2);
  sprite.setTextColor(TFT_WHITE, touchColor);
  sprite.drawString("TOUCH", TOUCH_BUTTON_X + BUTTON_WIDTH / 2,
                    BUTTON_Y + BUTTON_HEIGHT / 2, 2);
}

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

  float scale = 52.0f;
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

  sprite.setTextDatum(TC_DATUM);
  sprite.setTextColor(TFT_LIGHTGREY, TFT_NAVY);
  sprite.drawString("KEO DE XOAY", SCREEN_WIDTH / 2, 166, 2);
  drawNavigation();
  sprite.pushSprite(0, 0);
}

void updateAutoRotation() {
  uint32_t now = millis();
  float elapsedSeconds = (now - lastRotationUpdate) / 1000.0f;
  lastRotationUpdate = now;

  angleY += elapsedSeconds * AUTO_ROTATION_SPEED;
  if (angleY >= TWO_PI) {
    angleY -= TWO_PI;
  }
}

void resetMagicParticles() {
  for (MagicParticle &particle : magicParticles) {
    particle.position = lastTouch;
    particle.velocity = {0, 0};
    particle.life = 0;
  }
}

void spawnMagicParticles() {
  for (MagicParticle &particle : magicParticles) {
    if (particle.life != 0) {
      continue;
    }

    float angle = random(0, 628) / 100.0f;
    float speed = random(10, 35) / 10.0f;
    particle.position = lastTouch;
    particle.velocity = {static_cast<int16_t>(cosf(angle) * speed),
                         static_cast<int16_t>(sinf(angle) * speed)};
    particle.color = random(0, 2) == 0 ? TFT_CYAN : TFT_MAGENTA;
    particle.life = random(18, 36);
  }
}

void updateMagicParticles() {
  for (MagicParticle &particle : magicParticles) {
    if (particle.life == 0) {
      continue;
    }
    particle.position.x += particle.velocity.x;
    particle.position.y += particle.velocity.y;
    particle.velocity.y++;
    particle.life--;
  }
}

void drawTouchTestFrame() {
  sprite.fillSprite(TFT_BLACK);
  sprite.fillRoundRect(4, 4, SCREEN_WIDTH - 8, SCREEN_HEIGHT - 8, 12,
                       TFT_PURPLE);
  sprite.setTextDatum(TC_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_PURPLE);
  sprite.drawString("TOUCH MAGIC", SCREEN_WIDTH / 2, 12, 2);
  sprite.setTextColor(TFT_LIGHTGREY, TFT_PURPLE);
  sprite.drawString("CHAM VA DI CHUYEN", SCREEN_WIDTH / 2, 38, 2);

  sprite.fillCircle(lastTouch.x, lastTouch.y, 8, TFT_WHITE);
  sprite.drawCircle(lastTouch.x, lastTouch.y, 14, TFT_CYAN);
  for (const MagicParticle &particle : magicParticles) {
    if (particle.life != 0) {
      sprite.fillCircle(particle.position.x, particle.position.y, 2,
                        particle.color);
    }
  }

  sprite.setTextColor(TFT_LIGHTGREY, TFT_PURPLE);
  sprite.drawString("DIEM CHAM: " + String(lastTouch.x) + ", " +
                        String(lastTouch.y),
                    SCREEN_WIDTH / 2, 164, 2);
  drawNavigation();
  sprite.pushSprite(0, 0);
}

void changeMode(ScreenMode newMode) {
  if (screenMode == newMode) {
    return;
  }
  screenMode = newMode;
  if (screenMode == ScreenMode::TouchTest) {
    resetMagicParticles();
  }
}

void handleTouch() {
  bool isTouched = touch.tirqTouched() && touch.touched();
  if (!isTouched) {
    touchWasDown = false;
    return;
  }

  Vec2 point = readTouchPoint(touch.getPoint());
  if (!touchWasDown) {
    if (isInsideButton(point, CUBE_BUTTON_X, BUTTON_WIDTH)) {
      changeMode(ScreenMode::Cube);
    } else if (isInsideButton(point, TOUCH_BUTTON_X, BUTTON_WIDTH)) {
      changeMode(ScreenMode::TouchTest);
    }
  }

  if (screenMode == ScreenMode::TouchTest) {
    lastTouch = point;
    spawnMagicParticles();
  } else if (touchWasDown &&
             !isInsideButton(point, CUBE_BUTTON_X, BUTTON_WIDTH) &&
             !isInsideButton(point, TOUCH_BUTTON_X, BUTTON_WIDTH)) {
    int16_t dx = point.x - lastTouch.x;
    int16_t dy = point.y - lastTouch.y;
    angleY += dx * TOUCH_ROTATE_SENSITIVITY;
    angleX += dy * TOUCH_ROTATE_SENSITIVITY;
    angleX = constrain(angleX, -1.4f, 1.4f);
  }
  lastTouch = point;
  touchWasDown = true;
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
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

  touchscreenSPI.begin(TOUCH_SCLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN,
                       TOUCH_CS_PIN);
  touch.begin(touchscreenSPI);
  touch.setRotation(1);
  randomSeed(micros());
  lastRotationUpdate = millis();
  drawCubeFrame();
  Serial.println("Cube ready");
}

void loop() {
  handleTouch();
  if (screenMode == ScreenMode::Cube) {
    if (!touchWasDown) {
      updateAutoRotation();
    } else {
      lastRotationUpdate = millis();
    }
    drawCubeFrame();
  } else {
    updateMagicParticles();
    drawTouchTestFrame();
  }
  delay(20);
}
