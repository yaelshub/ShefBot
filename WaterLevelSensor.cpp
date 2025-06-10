  #include "WaterLevelSensor.h"

  WaterLevelSensor::WaterLevelSensor(uint8_t power, uint8_t sensor)
    : powerPin(power), readPin(sensor) {}

  void WaterLevelSensor::begin() {
  pinMode(powerPin, OUTPUT);         
  digitalWrite(powerPin, HIGH);   
  }

  int WaterLevelSensor::read() {
  return analogRead(readPin);  
  }

  