// #include <Arduino.h>
// #include <TFT_eSPI.h>
// #include <WiFi.h>
// #include <time.h>

// // Edit these two values before uploading the program.
// const char *WIFI_SSID = "Caffee_2_dua_minh";
// const char *WIFI_PASSWORD = "Haianhem123@";

// constexpr int SCREEN_WIDTH = 240;
// constexpr int SCREEN_HEIGHT = 320;
// constexpr char NTP_SERVER_1[] = "pool.ntp.org";
// constexpr char NTP_SERVER_2[] = "time.nist.gov";

// TFT_eSPI tft;
// TFT_eSprite sprite(&tft);

// const char *const WEEKDAYS[] = {
//     "CHU NHAT", "THU HAI", "THU BA", "THU TU",
//     "THU NAM", "THU SAU", "THU BAY"};

// void showMessage(const char *message, uint16_t color) {
//   sprite.fillSprite(TFT_BLACK);
//   sprite.setTextDatum(MC_DATUM);
//   sprite.setTextColor(color, TFT_BLACK);
//   sprite.drawString(message, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 2);
//   sprite.pushSprite(0, 0);
// }

// void connectWiFi() {
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

//   Serial.print("Connecting to Wi-Fi");
//   uint32_t start = millis();
//   while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
//     Serial.print('.');
//     showMessage("DANG KET NOI WI-FI", TFT_YELLOW);
//     delay(500);
//   }
//   Serial.println();
// }

// void drawClock() {
//   struct tm now;
//   if (!getLocalTime(&now, 1000)) {
//     showMessage("DANG DOI NTP...", TFT_YELLOW);
//     return;
//   }

//   char timeText[9];
//   char dateText[24];
//   snprintf(timeText, sizeof(timeText), "%02d:%02d:%02d",
//            now.tm_hour, now.tm_min, now.tm_sec);
//   snprintf(dateText, sizeof(dateText), "%02d/%02d/%04d",
//            now.tm_mday, now.tm_mon + 1, now.tm_year + 1900);

//   sprite.fillSprite(TFT_BLACK);
//   sprite.setTextDatum(MC_DATUM);
//   sprite.setTextColor(TFT_CYAN, TFT_BLACK);
//   sprite.drawString("THOI GIAN THUC", SCREEN_WIDTH / 2, 45, 2);
//   sprite.setTextColor(TFT_WHITE, TFT_BLACK);
//   sprite.drawString(timeText, SCREEN_WIDTH / 2, 135, 7);
//   sprite.setTextColor(TFT_GREEN, TFT_BLACK);
//   sprite.drawString(WEEKDAYS[now.tm_wday], SCREEN_WIDTH / 2, 215, 2);
//   sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
//   sprite.drawString(dateText, SCREEN_WIDTH / 2, 255, 4);
//   sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
//   sprite.drawString("NTP - UTC+7", SCREEN_WIDTH / 2, 295, 2);
//   sprite.pushSprite(0, 0);
// }

// void setup() {
//   Serial.begin(115200);
//   tft.init();
//   tft.setRotation(0);

//   sprite.setColorDepth(8);
//   if (!sprite.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
//     Serial.println("Cannot allocate TFT Sprite memory");
//     tft.fillScreen(TFT_RED);
//     return;
//   }

//   sprite.fillSprite(TFT_BLACK);
//   sprite.pushSprite(0, 0);
//   connectWiFi();

//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.println("Wi-Fi connected: " + WiFi.localIP().toString());
//     // POSIX timezone string for Vietnam: Indochina Time, UTC+7, no DST.
//     configTzTime("ICT-7", NTP_SERVER_1, NTP_SERVER_2);
//   } else {
//     Serial.println("Wi-Fi connection failed");
//   }
// }

// void loop() {
//   static uint32_t lastUpdate = 0;
//   if (millis() - lastUpdate >= 1000 || lastUpdate == 0) {
//     lastUpdate = millis();
//     drawClock();
//   }
//   delay(10);
// }

