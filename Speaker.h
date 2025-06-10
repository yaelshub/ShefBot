#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

class Speaker {
  private:
    DFRobotDFPlayerMini player;
    HardwareSerial& serial;

  public:
    Speaker(HardwareSerial& serialPort);
    bool begin();
    void play(uint8_t trackNumber);
    void stop();
    void setVolume(uint8_t volume);
};

#endif
