#include <i2c.h>
#define OLED_ENABLED
#ifdef OLED_ENABLED
  #include "SSD1306Ascii.h"
  #include "SSD1306AsciiWire.h"
  SSD1306AsciiWire oled;
  #define OLED_ADDR 0x3C
#endif

#define LED_D5 PD7
#define LED_D6 PD6
#define LED_D7 PD5

#define INPUT_SELECT PD3
#define RESET_ADV PD2

void _log_clear() {
  #ifdef OLED_ENABLED
    oled.clear();
  #endif
}

void _log(const char *msg, bool hex=false) {
  Serial.print(msg);
    #ifdef OLED_ENABLED
      oled.print(msg);
    #endif
}

void _logln(const char *msg, bool hex=false) {
  Serial.println(msg);
    #ifdef OLED_ENABLED
      oled.println(msg);
    #endif
}

template <typename T> void _log(T msg, bool hex=false){
  if(hex) {
    Serial.print(msg, HEX);
    #ifdef OLED_ENABLED
      oled.print(msg);
    #endif
  }
  else {
    Serial.print(msg);
    #ifdef OLED_ENABLED
      oled.print(msg);
    #endif
  }
}

template<typename T> void _logln(T msg, bool hex=false) {
  _log(msg, hex);
  Serial.println();
  #ifdef OLED_ENABLED
    oled.println();
  #endif
}