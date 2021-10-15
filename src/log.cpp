#include "log.h"
#include <i2c.h>

SSD1306AsciiWire oled;

SSD1306AsciiWire *getOled(){return &oled;}

void _log_clear() {
  #ifdef OLED_ENABLED
    oled.clear();
  #endif
}

void _log(const char *msg, bool hex) {
  Serial.print(msg);
    #ifdef OLED_ENABLED
      getOled()->print(msg);
    #endif
}

void _logln(const char *msg, bool hex) {
  Serial.println(msg);
    #ifdef OLED_ENABLED
      getOled()->println(msg);
    #endif
}

void _log(byte msg, bool hex) {
  Serial.print(msg);
    #ifdef OLED_ENABLED
      getOled()->print(msg);
    #endif
}

void _logln(byte msg, bool hex) {
  Serial.println(msg);
    #ifdef OLED_ENABLED
      getOled()->println(msg);
    #endif
}