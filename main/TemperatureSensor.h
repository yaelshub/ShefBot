#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor 
{
  private:
    uint8_t dataPin;
    OneWire oneWire;
    DallasTemperature sensor;

  public:
    TemperatureSensor(uint8_t pin);
    void begin();
    float readCelsius();  
};

#endif
