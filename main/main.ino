#include <ESP32Servo.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
// #include "servo.h"
#include "TemperatureSensor.h"
#include "WaterLevelSensor.h"
#include "Speaker.h"


#define TEMP_PIN         4
#define WATER_PWR_PIN    18
#define WATER_READ_PIN   34
#define SERVO_PIN        2
#define LED_PIN          5
#define NUM_LEDS         16

const char* ssid = "OLAX-4G-852E";
const char* password = "43471721";

// MyServo lidServo(SERVO_PIN);
Servo lidServo;
TemperatureSensor tempSensor(TEMP_PIN);
WaterLevelSensor waterSensor(WATER_PWR_PIN, WATER_READ_PIN);
Adafruit_NeoPixel fireLeds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);
Speaker speaker(Serial2);

enum RiceRecipeState 
{
  // 0 לא עושים כלום
  RECIPE_IDLE,
  PLAYING_AUDIO_SEQUENCE,
  WAITING_FOR_BOILING,
  MONITORING_WATER,
  // 4 המתכון הסתיים
  RECIPE_COMPLETED
};

const unsigned long AUDIO_INTERVALS[] = {0, 10000, 20000, 40000, 50000, 60000};
const float BOILING_TEMP = 90.0;
const int LOW_WATER_THRESHOLD = 20;
const int FIRE_LOW_INTENSITY = 80;
const int LID_OPEN_ANGLE = 180;

unsigned long timerStartMillis = 0;
unsigned long timerDuration = 0;
unsigned long recipeStartTime = 0;

int currentAudioStep = 1;
int cupsCount = 0;

bool timerActive = false;
bool isRecipeActive = false;
bool inRiceRecipeMode = false;
bool fireStarted = false;

RiceRecipeState currentState = RECIPE_IDLE;

void setCorsHeaders() 
{
  //כל דומיין יכול לשלוח לי בקשות
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptionsRequest() {
  setCorsHeaders();
  server.send(204);
}

void extinguishFire() {
  for (int i = 0; i < NUM_LEDS; i++) 
  {
    fireLeds.setPixelColor(i, 0);
  }
  fireLeds.show();
  fireStarted = false;

}

void igniteFire(uint8_t brightness = 255) 
{
  for (int i = 0; i < NUM_LEDS; i++) 
  {
    uint8_t red = brightness;
    uint8_t green = random(0, 20);
    uint8_t blue = random(0, 1);
    fireLeds.setPixelColor(i, fireLeds.Color(red, green, blue));
  }
  fireLeds.show();
  fireStarted = true;

}
// מפעילה קובץ אודיו לפי מספר הכוסות
void playAudioFile(int fileNumber) 
{
  speaker.playFolder(cupsCount, fileNumber);
}
//קוראת את רמת המים ומחזירה ערך 0–1000
int readWaterLevel() 
{
  digitalWrite(WATER_PWR_PIN, HIGH);
  delay(10);
  int raw = analogRead(WATER_READ_PIN);
  digitalWrite(WATER_PWR_PIN, LOW);
  int level = map(raw, 4095, 0, 0, 1000);
  level = constrain(level, 0, 1000);
  return level;
}

bool isTimeForNextAudio() 
{
  return (currentAudioStep <= 5 && millis() - recipeStartTime >= AUDIO_INTERVALS[currentAudioStep]);
}

void completeRecipe() 
{
  extinguishFire();
  lidServo.write(LID_OPEN_ANGLE);
  playAudioFile(6);
  currentState = RECIPE_COMPLETED;
  isRecipeActive = false;
  inRiceRecipeMode = false;
}

//התחלת המתכון, הדלקת אש, והשמעת קובץ ראשון
void initializeRecipe(int cups) 
{
  cupsCount = cups;
  currentState = PLAYING_AUDIO_SEQUENCE;
  recipeStartTime = millis();
  igniteFire(255);
  currentAudioStep = 1;
  isRecipeActive = true;
  inRiceRecipeMode = true;
  playAudioFile(1);
  currentAudioStep++;
}

//שלב ההשמעה ההדרגתית של קבצים
void handleAudioSequence() 
{
  if (isTimeForNextAudio()) 
  {
    playAudioFile(currentAudioStep);
    currentAudioStep++;
    if (currentAudioStep > 5) 
    {
      currentState = WAITING_FOR_BOILING;
    }
  }
}

//בדיקת רתיחה
void handleBoilingCheck() 
{
  float currentTemp = tempSensor.readCelsius();
  if (currentTemp >= BOILING_TEMP) 
  {
    igniteFire(FIRE_LOW_INTENSITY);
    currentState = MONITORING_WATER;
  }
}

//בדיקה האם נגמרו המים
void handleWaterMonitoring() 
{
  int level = readWaterLevel();
  if (level < LOW_WATER_THRESHOLD) 
  {
    extinguishFire();
    completeRecipe();
  }
}

void runRiceRecipe() 
{
  switch (currentState) 
  {
    case PLAYING_AUDIO_SEQUENCE: handleAudioSequence(); break;
    case WAITING_FOR_BOILING: handleBoilingCheck(); break;
    case MONITORING_WATER: handleWaterMonitoring(); break;
    default: break;
  }
}

void setup() 
{
  Serial.begin(115200);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("מנסה להתחבר ל-WiFi");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi התחבר!");
  Serial.print("כתובת IP: ");
  Serial.println(WiFi.localIP());

  // DFPlayer
  Serial.println("מנסה להפעיל DFPlayer...");
  if (!speaker.begin())
  {
    Serial.println("DFPlayer לא התחיל (Speaker init failed)");
  } 
  else 
  {
    Serial.println("DFPlayer התחיל בהצלחה");
  }

  pinMode(WATER_PWR_PIN, OUTPUT);
  digitalWrite(WATER_PWR_PIN, LOW);

  fireLeds.begin();
  fireLeds.show();
  lidServo.attach(SERVO_PIN,500,2500); 
  lidServo.write(90);
  tempSensor.begin();
  waterSensor.begin();

  server.on("/status", HTTP_GET, [&]() 
  {
    setCorsHeaders();
    String json = "{\"temp\":" + String(tempSensor.readCelsius()) +
                  ",\"water\":" + String(readWaterLevel()) +
                  ",\"fire\":\"" + String(fireStarted ? "on" : "off") + "\"}";
    server.send(200, "application/json", json);
  });

  server.on("/start-timer", HTTP_POST, []() 
  {
    setCorsHeaders();
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error || !doc.containsKey("minutes")) {
      Serial.println("Invalid JSON or missing 'minutes'");
      server.send(400, "text/plain", "Invalid JSON or missing 'minutes'");
      return;
    }
    timerDuration = doc["minutes"].as<int>() * 60000;
    timerStartMillis = millis();
    timerActive = true;
    fireStarted = false;
    extinguishFire();
    Serial.print("טיימר התחיל: ");
    Serial.print(doc["minutes"].as<int>());
    server.send(200, "text/plain", "Timer started");
  });

  server.on("/stop-timer", HTTP_POST, []() 
  {
   setCorsHeaders();
   timerActive = false;
   fireStarted = false;
   extinguishFire();
   lidServo.write(180);
   speaker.setVolume(30);
   speaker.playFolder(1, 6);  
  server.send(200, "text/plain", "Timer stopped");
  });

  server.on("/start-rice", HTTP_POST, []() {
    setCorsHeaders();
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error || !doc.containsKey("cups")) {
      server.send(400, "text/plain", "Invalid JSON or missing 'cups'");
      return;
    }
    initializeRecipe(doc["cups"]);
    server.send(200, "text/plain", "Rice recipe started");
  });

  server.on("/cancel-rice", HTTP_POST, []() 
  {
    setCorsHeaders();
    extinguishFire();
    isRecipeActive = false;
    inRiceRecipeMode = false;
    server.send(200, "text/plain", "Rice recipe canceled");
  });
  
  server.on("/rice-status", HTTP_GET, []() {
    setCorsHeaders();
    String status = "idle";
    if (inRiceRecipeMode && isRecipeActive) {
      if (currentState == PLAYING_AUDIO_SEQUENCE) status = "cooking";
      else if (currentState == WAITING_FOR_BOILING) status = "boiling";
      else if (currentState == MONITORING_WATER) status = "monitoring";
    } else if (currentState == RECIPE_COMPLETED) {
      status = "completed";
    }
    String json = "{\"status\":\"" + status + "\"}";
    server.send(200, "application/json", json);
  });
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      handleOptionsRequest();
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });

  server.on("/start-timer", HTTP_OPTIONS, handleOptionsRequest);
  server.on("/stop-timer", HTTP_OPTIONS, handleOptionsRequest);
  server.on("/start-rice", HTTP_OPTIONS, handleOptionsRequest);
  server.on("/cancel-rice", HTTP_OPTIONS, handleOptionsRequest);
  server.on("/status", HTTP_OPTIONS, handleOptionsRequest);

  server.begin();
}

void loop() 
{
  server.handleClient();
//אם התחיל המתכון של האורז
  if (inRiceRecipeMode && isRecipeActive) 
  {
    runRiceRecipe();
    return;
  }

  if (timerActive) 
  {
    //אם האש עדיין לא הודלקה
    if (!fireStarted) 
    {
      igniteFire(120);
      //מונע הדלקה נוספת בכל סבב של לופ 
      fireStarted = true;
    }
    //אם עבר הזמן
    if (millis() - timerStartMillis >= timerDuration) 
    {
      timerActive = false;
      fireStarted = false;
      extinguishFire();
      lidServo.write(180);
      speaker.setVolume(30);
      speaker.playFolder(1, 6);
    }
  }
}
