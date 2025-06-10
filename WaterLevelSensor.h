#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include <Arduino.h>

class WaterLevelSensor {
  private:
    uint8_t powerPin;
    uint8_t readPin;

  public:
    WaterLevelSensor(uint8_t power, uint8_t sensor);
    void begin();
    int read();
};

#endif

