// #include <SPI.h>
// #include <TFT_eSPI.h>
// #include <XPT2046_Touchscreen.h>

// #define TOUCH_CS  33
// #define TOUCH_IRQ 34

// TFT_eSPI tft = TFT_eSPI();
// XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// int timerMinutes = 1;
// bool inMainMenu = true;
// bool inTimerMenu = false;
// bool inRecipeMenu = false;
// bool inAmountSelection = false;
// bool inStartCooking = false;
// bool showingRecipe = false;
// bool countdownStarted = false;
// int cupAmount = 0;

// unsigned long countdownStartTime;
// unsigned long countdownDuration;

// void setup() {
//   Serial.begin(115200);
//   tft.init();
//   tft.setRotation(1);
//   ts.begin();
//   ts.setRotation(1);
//   tft.fillScreen(TFT_BLACK);
//   drawMainMenu();
// }

// void loop() {
//   if (ts.touched()) {
//     delay(150);
//     TS_Point p = ts.getPoint();
//     int x = map(p.x, 200, 3800, 0, tft.width());
//     int y = map(p.y, 200, 3600, 0, tft.height());

//     if (inMainMenu) {
//       if (x > 40 && x < 140 && y > 60 && y < 120) {
//         inMainMenu = false;
//         inTimerMenu = true;
//         drawTimerMenu();
//       } else if (x > 180 && x < 280 && y > 60 && y < 120) {
//         inMainMenu = false;
//         inRecipeMenu = true;
//         drawRecipeMenu();
//       }
//     } else if (inTimerMenu) {
//       if (x > 90 && x < 120 && y > 90 && y < 120) {
//         timerMinutes++;
//         drawTimerMenu();
//       } else if (x > 90 && x < 120 && y > 150 && y < 180) {
//         if (timerMinutes > 1)
//           timerMinutes--;
//         drawTimerMenu();
//       } else if (x > 140 && x < 200 && y > 190 && y < 230) {
//         countdownStarted = true;
//         countdownStartTime = millis();
//         countdownDuration = timerMinutes * 60 * 1000;
//         tft.fillScreen(TFT_BLACK);
//         inTimerMenu = false;
//       }
//     } else if (inRecipeMenu) {
//       if (x > 40 && x < 140 && y > 60 && y < 120) {
//         inRecipeMenu = false;
//         inAmountSelection = true;
//         drawCupSelection();
//       } else if (x > 180 && x < 280 && y > 60 && y < 120) {
//         inRecipeMenu = false;
//         showingRecipe = true;
//         showRecipe("onion");
//       } else if (x > 240 && x < 280 && y > 180 && y < 220) {
//         drawMainMenu();
//       }
//     } else if (inAmountSelection) {
//       if (x > 40 && x < 100 && y > 100 && y < 160) {
//         if (cupAmount > 0) cupAmount--;
//         drawCupSelection();
//       } else if (x > 180 && x < 240 && y > 100 && y < 160) {
//         cupAmount++;
//         drawCupSelection();
//       } else if (x > 100 && x < 180 && y > 200 && y < 240) {
//         inAmountSelection = false;
//         inStartCooking = true;
//         drawStartCooking();
//       }
//     } else if (inStartCooking) {
//       if (x > 100 && x < 220 && y > 100 && y < 180) {
//         inStartCooking = false;
//         showingRecipe = true;
//         showRecipe("rice");
//       }
//     }
//   }

//   if (countdownStarted) {
//     unsigned long elapsed = millis() - countdownStartTime;
//     if (elapsed >= countdownDuration) {
//       countdownStarted = false;
//       tft.fillScreen(TFT_RED);
//       tft.setTextColor(TFT_WHITE);
//       tft.setTextSize(3);
//       tft.drawString("Time's up!", 70, 100);
//       delay(2000);
//       drawMainMenu();
//     } else {
//       unsigned long remaining = countdownDuration - elapsed;
//       int seconds = (remaining / 1000) % 60;
//       int minutes = (remaining / 1000) / 60;
//       char buffer[10];
//       sprintf(buffer, "%02d:%02d", minutes, seconds);
//       tft.fillRect(100, 100, 120, 50, TFT_BLACK);
//       tft.setTextColor(TFT_GREEN, TFT_BLACK);
//       tft.setTextSize(4);
//       tft.drawString(buffer, 100, 100);
//       delay(500);
//     }
//   }
// }

// void drawMainMenu() {
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextSize(2);
//   tft.setTextColor(TFT_WHITE);
//   tft.fillRect(40, 60, 100, 60, TFT_RED);
//   tft.drawString("timer", 65, 80);
//   tft.fillRect(180, 60, 100, 60, TFT_RED);
//   tft.drawString("choose", 190, 75);
//   tft.drawString("a meal", 195, 95);
//   tft.setTextColor(TFT_RED);
//   tft.setTextSize(2);
//   tft.drawString("Bon appatit!!", 60, 160);
//   inMainMenu = true;
//   inTimerMenu = false;
//   inRecipeMenu = false;
//   inAmountSelection = false;
//   inStartCooking = false;
//   showingRecipe = false;
// }

// void drawTimerMenu() {
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextSize(2);
//   tft.setTextColor(TFT_WHITE);
//   tft.drawString("בחר דקות:", 100, 40);
//   tft.drawRect(100, 90, 40, 40, TFT_WHITE);
//   tft.drawString("+", 112, 100);
//   tft.drawRect(100, 150, 40, 40, TFT_WHITE);
//   tft.drawString("-", 112, 160);
//   tft.setTextSize(3);
//   tft.drawString(String(timerMinutes), 160, 115);
//   tft.setTextSize(2);
//   tft.fillRect(140, 190, 60, 40, TFT_YELLOW);
//   tft.setTextColor(TFT_BLACK);
//   tft.drawString("אישור", 145, 200);
//   tft.setTextColor(TFT_WHITE);
// }

// void drawRecipeMenu() {
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextSize(2);
//   tft.setTextColor(TFT_BLACK);
//   tft.fillRect(40, 60, 100, 60, TFT_CYAN);
//   tft.drawString("rice", 70, 80);
//   tft.fillRect(180, 60, 100, 60, TFT_CYAN);
//   tft.drawString("onion", 200, 75);
//   tft.drawString("soup", 205, 95);
//   tft.fillCircle(260, 200, 20, TFT_RED);
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(1);
//   tft.drawString("exit", 250, 195);
// }

// void drawCupSelection() {
//   tft.fillScreen(TFT_WHITE);
//   tft.setTextColor(TFT_RED);
//   tft.setTextSize(2);
//   tft.drawString("choose amount of cups", 10, 30);
//   tft.setTextSize(4);
//   tft.drawString(String(cupAmount), 140, 100);
//   tft.fillRect(40, 100, 60, 60, TFT_DARKGREY);
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(3);
//   tft.drawString("-", 65, 115);
//   tft.fillRect(180, 100, 60, 60, TFT_DARKGREY);
//   tft.drawString("+", 200, 115);
//   tft.setTextSize(2);
//   tft.fillRect(100, 200, 80, 40, TFT_BLUE);
//   tft.drawString("Enter", 110, 210);
// }

// void drawStartCooking() {
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(3);
//   tft.drawString("Rice", 120, 40);
//   tft.fillCircle(160, 130, 50, TFT_RED);
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(2);
//   tft.drawString("start", 140, 120);
//   tft.drawString("cooking", 130, 140);
// }

// void showRecipe(String name) {
//   tft.fillScreen(TFT_RED);
//   tft.setTextColor(TFT_BLACK);
//   tft.setTextSize(3);
//   if (name == "rice") {
//     tft.drawString("Rice", 120, 10);
//     tft.setTextSize(2);
//     tft.drawString("1 cup rice", 10, 50);
//     tft.drawString("2 tbsp olive oil", 10, 80);
//     tft.drawString("1 tsp salt", 10, 110);
//     tft.drawString("1.5 cups water", 10, 140);
//   } else if (name == "onion") {
//     tft.drawString("Onion Soup", 80, 10);
//     tft.setTextSize(2);
//     tft.drawString("3 onions", 10, 50);
//     tft.drawString("1/2 tsp salt", 10, 80);
//     tft.drawString("2 tbsp olive oil", 10, 110);
//     tft.drawString("15g butter", 10, 140);
//     tft.drawString("1 cup white wine", 10, 170);
//     tft.drawString("2 tbsp soy sauce", 10, 200);
//     tft.drawString("1 tbsp flour", 10, 230);
//     tft.drawString("6 cups water", 10, 260);
//   }
// }
