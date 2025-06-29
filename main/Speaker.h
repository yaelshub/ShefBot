#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

class Speaker 
{
  private:
  //אובייקט שאחראי על שליטה על המודול עצמו
    DFRobotDFPlayerMini player;
  //הפניה לפורט סידורי
    HardwareSerial& serial;

  public:
    Speaker(HardwareSerial& serialPort);
    bool begin();
    void play(uint8_t trackNumber);
    void stop();
    void setVolume(uint8_t volume);
    void playFolder(uint8_t folder, uint8_t file);
};

#endif