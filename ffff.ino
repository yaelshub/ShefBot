#include <Arduino.h>
#include "servo.h"
#include "TemperatureSensor.h"
#include "WaterLevelSensor.h"
#include "speaker.h"
#include <Adafruit_NeoPixel.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define TEMP_PIN         4
#define WATER_PWR_PIN    18
#define WATER_READ_PIN   34
#define SERVO_PIN        2
#define LED_PIN          5
#define NUM_LEDS         16

MyServo lidServo(SERVO_PIN);
TemperatureSensor tempSensor(TEMP_PIN);
WaterLevelSensor waterSensor(WATER_PWR_PIN, WATER_READ_PIN);
Speaker speaker(Serial2);
Adafruit_NeoPixel fireLeds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
TFT_eSPI tft = TFT_eSPI();

bool alreadyHandled = false;
bool cookingPaused = false;
bool timerActive = false;
unsigned long timerStartMillis = 0;
unsigned long timerDuration = 0;

// --- טיימר דינמי ---
bool timerSelectMode = false;
int selectedMinutes = 5;

// --- DEBUG MODE ---
bool debugMode = true; // למצב פיתוח

// --- אש ---
void igniteFire(uint8_t brightness = 255) {
  Serial.println(" מדליק אש!");
  for (int i = 0; i < NUM_LEDS; i++) {
    fireLeds.setPixelColor(i, fireLeds.Color(brightness, random(brightness / 2, brightness), 0));
  }
  fireLeds.show();
}

void extinguishFire() {
  Serial.println(" מכבה אש!");
  for (int i = 0; i < NUM_LEDS; i++) {
    fireLeds.setPixelColor(i, 0);
  }
  fireLeds.show();
}

// --- בדיקת מסך ---
void testScreen() {
  Serial.println(" בודק מסך...");
  tft.fillScreen(TFT_RED);
  delay(500);
  tft.fillScreen(TFT_GREEN);
  delay(500);
  tft.fillScreen(TFT_BLUE);
  delay(500);
  tft.fillScreen(TFT_BLACK);
  Serial.println(" מסך עובד!");
}

// --- תצוגה ראשית ---
void showControls() {
  tft.fillRect(10, 180, 100, 40, TFT_RED);
  tft.drawRect(10, 180, 100, 40, TFT_WHITE);
  tft.setCursor(20, 190);
  tft.setTextColor(TFT_WHITE);
  tft.print("הפסק");

  tft.fillRect(120, 180, 100, 40, TFT_BLUE);
  tft.drawRect(120, 180, 100, 40, TFT_WHITE);
  tft.setCursor(130, 190);
  tft.setTextColor(TFT_WHITE);
  tft.print("טיימר");
}

void showMainMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, 10);
  tft.println("מערכת בישול חכמה");
  
  // הצגת סטטוס
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 40);
  tft.print("טמפ: ");
  tft.print(tempSensor.readCelsius());
  tft.print("C");
  
  tft.setCursor(10, 60);
  tft.print("מים: ");
  tft.print(waterSensor.read());
  
  tft.setCursor(10, 80);
  tft.print("אש: ");
  tft.print((tempSensor.readCelsius() > 25 && waterSensor.read() < 4000) ? "דולקת" : "כבויה");
  
  tft.setTextSize(2);
  tft.setCursor(10, 140);
  tft.setTextColor(TFT_WHITE);
  tft.println("בחרו פעולה:");
}

// --- תפריט טיימר דינמי ---
void showTimerSelector() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  tft.setCursor(30, 30);
  tft.println("בחר זמן לטיימר:");

  // תצוגת דקות
  tft.fillRect(90, 70, 100, 40, TFT_DARKGREY);
  tft.drawRect(90, 70, 100, 40, TFT_WHITE);
  tft.setCursor(120, 80);
  tft.setTextColor(TFT_YELLOW);
  tft.print(selectedMinutes);

  // + 
  tft.fillRect(200, 70, 40, 40, TFT_GREEN);
  tft.drawRect(200, 70, 40, 40, TFT_WHITE);
  tft.setCursor(210, 80);
  tft.setTextColor(TFT_BLACK);
  tft.print("+");

  // -
  tft.fillRect(40, 70, 40, 40, TFT_RED);
  tft.drawRect(40, 70, 40, 40, TFT_WHITE);
  tft.setCursor(50, 80);
  tft.setTextColor(TFT_BLACK);
  tft.print("-");

  // אישור
  tft.fillRect(90, 130, 100, 40, TFT_BLUE);
  tft.drawRect(90, 130, 100, 40, TFT_WHITE);
  tft.setCursor(110, 140);
  tft.setTextColor(TFT_WHITE);
  tft.print("אישור");
}

// --- טיפול במגע ---
void handleTouch() {
  uint16_t x, y;
  if (tft.getTouch(&x, &y)) {
    Serial.print(" נגיעה ב: X="); Serial.print(x); Serial.print(" Y="); Serial.println(y);
    
    if (timerSelectMode) {
      if (x > 200 && x < 240 && y > 70 && y < 110) {
        if (selectedMinutes < 99) selectedMinutes++;
        showTimerSelector();
      } else if (x > 40 && x < 80 && y > 70 && y < 110) {
        if (selectedMinutes > 1) selectedMinutes--;
        showTimerSelector();
      } else if (x > 90 && x < 190 && y > 130 && y < 170) {
        timerStartMillis = millis();
        timerDuration = selectedMinutes * 60000UL;
        timerActive = true;
        timerSelectMode = false;
        delay(100);
        showMainMenu();
        showControls();
      }
    } else {
      if (x > 10 && x < 110 && y > 180 && y < 220) {
        cookingPaused = true;
        extinguishFire();
        speaker.play(6); // "הבישול הופסק"
        delay(100);
      }

      if (x > 120 && x < 220 && y > 180 && y < 220) {
        timerSelectMode = true;
        showTimerSelector();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("מערכת בישול חכמה מתחילה...");
  pinMode(WATER_PWR_PIN, OUTPUT);
  digitalWrite(WATER_PWR_PIN, LOW); // כיבוי ראשוני

  // הפעלת רכיבים בזה אחר זה
  Serial.println(" מפעיל חיישנים...");
  lidServo.attach();
  tempSensor.begin();
  waterSensor.begin();
  pinMode(WATER_PWR_PIN, INPUT);

  Serial.println("מפעיל לדים...");
  fireLeds.begin();
  fireLeds.show();
  
  // בדיקת לדים
  Serial.println("בודק לדים...");
  for(int i = 0; i < NUM_LEDS; i++) {
    fireLeds.setPixelColor(i, fireLeds.Color(50, 0, 0));
    fireLeds.show();
    delay(50);
  }
  extinguishFire();
  
  Serial.println("מפעיל מסך...");
  tft.init();
  tft.setRotation(1);
  
  // בדיקת מסך
  if (debugMode) {
    testScreen();
  }
  
  showMainMenu();
  showControls();
  
  Serial.println("מפעיל רמקול...");
  if (!speaker.begin()) {
    Serial.println(" DFPlayer לא זמין");
  } else {
    Serial.println("DFPlayer מוכן");
    speaker.setVolume(80); // נמוך יותר בהתחלה
    delay(500);
    speaker.play(1); // "המערכת הופעלה"
    delay(1000);
  }

  Serial.println("המערכת מוכנה!");
}

void loop() {
  float temp = tempSensor.readCelsius();
  int water = analogRead(WATER_READ_PIN); 

  Serial.print(" Temp: "); 
  Serial.print(temp);
  Serial.print("°C | Water: "); 
  Serial.println(water);

  // עדכון תצוגה כל 5 שניות
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    showMainMenu();
    showControls();
    lastUpdate = millis();
  }

  if (cookingPaused) {
    handleTouch();
    delay(300);
    return;
  }

  // תנאים מותאמים למצב פיתוח
  bool shouldIgnite = false;
  if (debugMode) {
    // במצב פיתוח: תנאים קלים יותר
    shouldIgnite = (temp > 25 && water < 4000); // טמפ מעל 25 ומים לא מקסימלי
  } else {
    // במצב רגיל: תנאים מקוריים
    shouldIgnite = (temp > 40 && water >= 300);
  }

  if (shouldIgnite) {
    igniteFire();
  } else {
    extinguishFire();
  }

  // בדיקת מים בטמפרטורה גבוהה
  if (temp >= 100 && !alreadyHandled) {
    if (water < 300) {
      extinguishFire();
      lidServo.write(90);
      speaker.play(6); // "אין מים – כיבוי אוטומטי"
      delay(100);
      alreadyHandled = true;
    }
  }

  if (temp < 95) {
    alreadyHandled = false;
    lidServo.write(0);
  }

  // ניהול טיימר דינמי
  if (timerActive && millis() - timerStartMillis >= timerDuration) {
    timerActive = false;
    extinguishFire();
    speaker.play(6); // "הטיימר הסתיים - הבישול הופסק"
    delay(100);
  }

  handleTouch();
  delay(1000);
}