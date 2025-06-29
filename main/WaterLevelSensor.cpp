  #include "WaterLevelSensor.h"

  WaterLevelSensor::WaterLevelSensor(uint8_t power, uint8_t sensor)
    : powerPin(power), readPin(sensor) {}

  void WaterLevelSensor::begin() {
    // כדכי לתת חשמל לחיישן הופך את הפין לפלט
  pinMode(powerPin, OUTPUT); 
  //מדליק את החיישן        
  digitalWrite(powerPin, HIGH);   
  }

  int WaterLevelSensor::read() {
  return analogRead(readPin);  
  }

  