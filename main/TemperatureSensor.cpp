#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(uint8_t pin):
 dataPin(pin), oneWire(pin), sensor(&oneWire) {}


void TemperatureSensor::begin() 
{
  sensor.begin();
}

float TemperatureSensor::readCelsius() 
{
  //בקשה לביצוע מדידה
  sensor.requestTemperatures();
  float tempC = sensor.getTempCByIndex(0);
  return tempC;
}
