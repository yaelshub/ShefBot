#include "servo.h"

MyServo::MyServo(int pin) 
{
  servoPin = pin;
}

void MyServo::attach() 
{
  pinMode(servoPin, OUTPUT);
}

void MyServo::write(int angle) 
{
  // ממיר את הזווית הרצויה לרוחב פולס במיקרו־שניות עבור הסרוו
  int pulseWidth = map(angle, 0, 180, 500, 2500); 

  // שליחה של לפחות 50 פולסים
  for (int i = 0; i < 50; i++) {
    digitalWrite(servoPin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(servoPin, LOW);
    delay(20);  
  }
}


