#include "servo.h"

MyServo::MyServo(int pin) {
  servoPin = pin;
}

void MyServo::attach() {
  pinMode(servoPin, OUTPUT);
}

void MyServo::write(int angle) {
  int pulseWidth = map(angle, 0, 180, 500, 2500); 
  digitalWrite(servoPin, HIGH);
  delayMicroseconds(pulseWidth);
  digitalWrite(servoPin, LOW);
  delay(20);  
}
