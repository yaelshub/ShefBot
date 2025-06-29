#ifndef MY_SERVO_H
#define MY_SERVO_H

#include <Arduino.h>

class MyServo 
{
  private:
    int servoPin;

  public:
    MyServo(int pin);
    void attach();
    void write(int angle);
};

#endif

