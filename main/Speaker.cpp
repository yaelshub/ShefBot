#include "Speaker.h"

Speaker::Speaker(HardwareSerial& serialPort) : serial(serialPort) {}

bool Speaker::begin() {
  serial.begin(9600, SERIAL_8N1, 27, 26);
   Serial.println("Trying to start DFPlayer...");
  if (!player.begin(serial)) {
    Serial.println("שגיאה: DFPlayer לא התחיל");
    return false;
  }

  player.volume(30); 
  Serial.println("DFPlayer התחיל בהצלחה");
  return true;
}

void Speaker::play(uint8_t trackNumber) {
  player.play(trackNumber);
}

void Speaker::stop() {
  player.stop();
}

void Speaker::setVolume(uint8_t volume) {
  player.volume(volume);
}

void Speaker::playFolder(uint8_t folder, uint8_t file) {
  player.playFolder(folder, file);
}
