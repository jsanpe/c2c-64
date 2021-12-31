#pragma once

#include "config.h"
#ifdef OLED_ENABLED
  #include "SSD1306Ascii.h"
  #include "SSD1306AsciiWire.h"
  #define OLED_ADDR 0x3C
#endif

class UI
{
    SSD1306AsciiWire _oled;
    void initOled();
    public:
    void status_mid(bool cvbs, bool locked, bool interlaced, bool ntsc, 
                    bool pal, bool secam, bool hz60, byte profile);
    void status_top(bool cvbs, bool locked, bool interlaced, bool ntsc, 
                    bool pal, bool secam, bool hz60, byte profile);
    void initUI();
    SSD1306AsciiWire *oled(){return &_oled;}
    void info(const __FlashStringHelper* msg);
    void info(const char* msg);
    void info(const char* msg, byte val1, byte val2);
    void printMenuItem(int8_t position, const __FlashStringHelper* name, const __FlashStringHelper* value);
    void printMenuItem(int8_t position, String name, String value);
    void logo();
    void clearUI();
    void health();

    byte getUserInput();
};



