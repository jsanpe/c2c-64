#include "log.h"
#include "Arduino.h"

#define TOP_STATUS_SOURCE_LEN 6
#define TOP_STATUS_STANDARD_LEN 7
#define TOP_STATUS_SIGNAL_LEN 6


#define MID_STATUS_INPUT_LEN 7
#define MID_STATUS_STANDARD_LEN 16
#define MID_STATUS_PROFILE_LEN 16

void status_top(bool cvbs, bool locked, bool interlaced, bool ntsc, bool pal, bool secam, bool hz60) {
    SSD1306AsciiWire *oled = getOled();
    byte fontW = oled->fontWidth()+1;
    byte fontH = 0;//oled->fontRows();
    byte currentCol = 0;
    oled->clearField(0, fontH, TOP_STATUS_SOURCE_LEN);
    oled->print(cvbs?F("Cvbs"):F("Svideo"));
    currentCol += TOP_STATUS_SOURCE_LEN*fontW;
    
    if(locked){
        oled->clearField(currentCol, fontH, TOP_STATUS_STANDARD_LEN);
        oled->print(ntsc?F("  NTSC "):(pal?F("  PAL  "):F(" SECAM ")));
        currentCol += TOP_STATUS_STANDARD_LEN*fontW;
        oled->clearField(currentCol, fontH, TOP_STATUS_SIGNAL_LEN);
        oled->print(hz60?F("60h"):F("50h"));
        oled->print(interlaced?F("(i)"):F("(p)"));
    } else {
        oled->clearField(currentCol, fontH, TOP_STATUS_STANDARD_LEN+TOP_STATUS_SIGNAL_LEN);
        oled->print(F(" - No Signal"));
    }
}

void printByte(byte v) {
    SSD1306AsciiWire *oled = getOled();
    char units = '0' + (v%10);
    char tenths = '0' + (v/10);
    oled->print(tenths); oled->print(units);
}

void status_mid(bool cvbs, bool locked, bool interlaced, bool ntsc, bool pal, bool secam, bool hz60, byte profile) {
    SSD1306AsciiWire *oled = getOled();
    byte fontW = oled->fontWidth()+1;
    byte fontH = 0;//oled->fontRows();
    
    //input
    oled->clearField(0, 0, (MID_STATUS_INPUT_LEN));oled->print("-Input-");
    oled->clearField(0, 1, (MID_STATUS_INPUT_LEN));oled->print(cvbs?F(" CVBS"):F("SVideo"));
    oled->clearField(0, 2, (MID_STATUS_INPUT_LEN));oled->print("-------");
    oled->clearField(0, 3, (MID_STATUS_INPUT_LEN));
        oled->print(hz60?F("60h"):F("50h"));oled->print(interlaced?F("(i)"):F("(p)"));

    if(locked){
        oled->set2X();
        oled->clearField(MID_STATUS_INPUT_LEN*fontW, 1, MID_STATUS_STANDARD_LEN);
        oled->print(ntsc?F(" NTSC "):(pal?F("  PAL  "):F(" SECAM ")));
        oled->set1X();
    } else {
        //oled->set2X();
        oled->clearField(MID_STATUS_INPUT_LEN*fontW, 1, MID_STATUS_STANDARD_LEN);
        oled->print(F("  Signal "));
        oled->print(F("not found"));
        //oled->set1X();
    }

    oled->clearField(0, 5, MID_STATUS_PROFILE_LEN);
    oled->print("Profile:");printByte(profile);
}

char phaseToChar(byte phase){
    switch(phase) {
        case 0: return '|';
        case 1: return '/';
        case 2: return '-';
        case 3: return '\\';
    }
    return '*';
}
void status_health() {
    static byte phase=0;
    phase = (phase+1)%4;
    SSD1306AsciiWire *oled = getOled();
    byte fontW = oled->fontWidth()+1;
    byte fontH = 0;
    oled->clearField(20*fontW, fontH, 1);
    oled->print(phaseToChar(phase));
}


void logo() {
    #define LOGO_LN1 F(".::::. .:::::. .::::.")
    #define LOGO_LN2 F(":+  +: :+: :+: :+  +:")
    #define LOGO_LN3 F("+:        +:+  +:    ")
    #define LOGO_LN4 F("+#       +#+   +#    ")
    #define LOGO_LN5 F("#+  +#  #+#    #+  +#")
    #define LOGO_LN6 F("`####' ####### `####'")

    SSD1306AsciiWire *oled = getOled();
    oled->clear();
    oled->setCursor(0,0);
    oled->println(LOGO_LN1);
    oled->println(LOGO_LN2);
    oled->println(LOGO_LN3);
    oled->println(LOGO_LN4);
    oled->println(LOGO_LN5);
    oled->println(LOGO_LN6);
}

void info(const char *msg) {
    SSD1306AsciiWire *oled = getOled();
    oled->clearField(0, 7, 25);
    oled->print(msg);
}