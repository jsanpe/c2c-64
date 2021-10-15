#pragma once
#include "Arduino.h"
#include "config.h"

#ifdef OLED_ENABLED
  #include "SSD1306Ascii.h"
  #include "SSD1306AsciiWire.h"
  #define OLED_ADDR 0x3C
#endif

void _log_clear();

SSD1306AsciiWire *getOled();

void _log(const char *msg, bool hex=false);
void _logln(const char *msg, bool hex=false);
void _log(byte msg, bool hex=false);
void _logln(byte msg, bool hex=false);
/*
template <typename T> void _log(T msg, bool hex=false){
  if(hex) {
    Serial.print(msg, HEX);
    #ifdef OLED_ENABLED
      getOled()->print(msg);
    #endif
  }
  else {
    Serial.print(msg);
    #ifdef OLED_ENABLED
      getOled()->print(msg);
    #endif
  }
}

template<typename T> void _logln(T msg, bool hex=false) {
  _log(msg, hex);
  Serial.println();
  #ifdef OLED_ENABLED
    getOled()->println();
  #endif
}
*/