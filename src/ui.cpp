#include "ui.h"
#include "i2c.h"
#include "mcurses.h"

#define MID_STATUS_INPUT_LEN 7
#define MID_STATUS_STANDARD_LEN 16
#define MID_STATUS_PROFILE_LEN 16

void addstrF(const __FlashStringHelper *ifsh)
{
  const char PROGMEM *p = (const char PROGMEM *)ifsh;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    addch(c);
  }
}

bool detect_oled(byte addr) {
    byte ret = send_i2c(addr, 0x00, 0xE3);
    Serial.print(ret==0?F("Oled detected"):F("Oled NOT detected"));
    return ret==0;
}

#define DEFAULT_FONT Adafruit5x7
#define MENU_ITEM_FONT lcd5x7

bool has_oled = false;
void UI::initOled() {
  #ifdef OLED_ENABLED
    has_oled = detect_oled(OLED_ADDR);
    SSD1306AsciiWire *oled = this->oled();
    has_oled=true;
    oled->begin(&Adafruit128x64, OLED_ADDR);
    oled->displayRemap(true);
    oled->setFont(DEFAULT_FONT);
    oled->clear();
    info("Oled screen found!",0,0);
  #endif
}

void UI::logo() {
    #define LOGO_LN1 F(".::::. .:::::. .::::.")
    #define LOGO_LN2 F(":+  +: :+: :+: :+  +:")
    #define LOGO_LN3 F("+:        +:+  +:    ")
    #define LOGO_LN4 F("+#       +#+   +#    ")
    #define LOGO_LN5 F("#+  +#  #+#    #+  +#")
    #define LOGO_LN6 F("`####' ####### `####'")

    SSD1306AsciiWire *oled = this->oled();
    oled->clear();
    oled->setCursor(0,1);
    oled->println(LOGO_LN1);
    oled->println(LOGO_LN2);
    oled->println(LOGO_LN3);
    //oled->println(LOGO_LN4);
    oled->println(LOGO_LN5);
    oled->println(LOGO_LN6);
}

void printByte(SSD1306AsciiWire *oled, byte v) {
    char units = '0' + (v%10);
    char tenths = '0' + (v/10);
    oled->print(tenths); oled->print(units);
}

void status_uart(bool cvbs, bool locked, bool interlaced, bool ntsc, 
                    bool pal, bool secam, bool hz60, byte profile) 
{
    mvaddstr(0,0, "Press (M) for Menu - (Up/Down) arrows for scrolling - (Enter) to Select - (S) to switch CVBS/S-Video");
    mvaddstr(1,0,"Source: ");
    attrset(cvbs?B_YELLOW:B_BLUE);
    addstr(cvbs?"CVBS    ":"S-Video ");attrset(B_BLACK);
    addstr("    ");

    addstr("Locked: ");
    attrset(locked?B_GREEN:B_RED);
    addch(locked?'Y':'N');attrset(B_BLACK);
    addstr("    ");

    if(locked) {
        addstr("Interlaced: ");
        attrset(interlaced?B_GREEN:B_RED);
        addch(interlaced?'Y':'N');attrset(B_BLACK);
        addstr("    ");
        addstr("Standard: ");addstr(ntsc?"  NTSC ":(pal?"  PAL  ":" SECAM "));
    }
}

void UI::status_top(bool cvbs, bool locked, bool interlaced, bool ntsc, 
                    bool pal, bool secam, bool hz60, byte profile) {
    if(has_oled) {
        SSD1306AsciiWire *oled = this->oled();
        byte fontW = oled->fontWidth()+1;
        byte fontH = 0;//oled->fontRows();
        
        //input
        oled->clearField(0, 0, (MID_STATUS_INPUT_LEN));oled->print(cvbs?F(" CVBS"):F("SVideo"));
        oled->clearField(MID_STATUS_INPUT_LEN*fontW, 0, MID_STATUS_STANDARD_LEN);
        if(locked) oled->print(ntsc?F("  NTSC "):(pal?F("  PAL  "):F(" SECAM ")));
        else oled->print(F("No Signal"));
    }

    status_uart(cvbs, locked, interlaced, ntsc, pal, secam, hz60, profile);
}

void UI::status_mid(bool cvbs, bool locked, bool interlaced, bool ntsc, 
                    bool pal, bool secam, bool hz60, byte profile) {
    if(has_oled) {
        SSD1306AsciiWire *oled = this->oled();
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
            oled->print(ntsc?F("  NTSC "):(pal?F("  PAL  "):F(" SECAM ")));
            oled->set1X();
        } else {
            //oled->set2X();
            oled->clearField(MID_STATUS_INPUT_LEN*fontW, 1, MID_STATUS_STANDARD_LEN);
            oled->print(F("   Signal "));
            oled->clearField(MID_STATUS_INPUT_LEN*fontW, 2, MID_STATUS_STANDARD_LEN);
            oled->print(F(" not found"));
            //oled->set1X();
        }

        oled->clearField(0, 5, MID_STATUS_PROFILE_LEN);
        oled->print("Profile:");printByte(oled, profile);
    }

    status_uart(cvbs, locked, interlaced, ntsc, pal, secam, hz60, profile);
}

void Arduino_putchar(uint8_t c)
{
  Serial.write(c);
}

char Arduino_getchar()
{
  if (!Serial.available()) return 'A';
  return Serial.read();
}

#include "buttons.h"
byte UI::getUserInput() {
    uint_fast8_t input = getch();
    if (input=='M' || input=='m') return MENU_BUTTON_PRESSED;
    if (input==KEY_DOWN) return PLUS_BUTTON_PRESSED;
    if (input==KEY_UP) return MINUS_BUTTON_PRESSED;
    if (input=='S') return SOURCE_BUTTON_PRESSED;
    if (input==KEY_CR) return SOURCE_BUTTON_PRESSED;
    return 0;
}

void UI::initUI(){
    this->initOled();
    setFunction_putchar(Arduino_putchar);
    setFunction_getchar(Arduino_getchar);
}

void UI::clearUI(){
    this->_oled.clear();
    clear();
}

#define INFO_MESSAGE_ROW 7
#define DEBUG_MESSAGE_ROW 8

#define INFO_MESSAGE_ROW_UART 10

void UI::info(const __FlashStringHelper* msg)
{
    if(has_oled) {
        SSD1306AsciiWire *oled = this->oled();
        oled->setCursor(0, INFO_MESSAGE_ROW);
        oled->print(msg);
    }

    move(INFO_MESSAGE_ROW_UART, 0);
    addstrF(msg);
}
void UI::info(const char* msg)
{
    if(has_oled){
        SSD1306AsciiWire *oled = this->oled();
        oled->setCursor(0, INFO_MESSAGE_ROW);
        oled->print(msg);
    }

    mvaddstr(INFO_MESSAGE_ROW_UART, 0, msg);
}

void print_byte(byte b) {
  char map[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  addstr("0x");addch(map[(b>>4)&0xf]);addch(map[b&0xf]);
}

void UI::info(const char* msg, byte val1, byte val2)
{
    if(has_oled){
        SSD1306AsciiWire *oled = this->oled();
        oled->setCursor(0, INFO_MESSAGE_ROW);
        oled->print(msg);
        oled->print(F(":"));
        oled->print(val1);
        oled->print(F("|"));
        oled->print(val2);
    }

    move(INFO_MESSAGE_ROW_UART, 0);
    addstr(msg);
    addstr(":");
    print_byte(val1);
    addstr("|");
    print_byte(val2);
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

void UI::health() {
    static byte phase=0;
    phase = (phase+1)%4;
    SSD1306AsciiWire *oled = this->oled();
    byte fontW = oled->fontWidth()+1;
    byte fontH = 0;
    oled->clearField(20*fontW, fontH, 1);
    oled->print(phaseToChar(phase));
}

#define MENU_ROW 3
#define MENU_ROW_UART 5

void UI::printMenuItem(int8_t position, const __FlashStringHelper* name, const __FlashStringHelper* value)
{
    if(has_oled) {
        SSD1306AsciiWire *oled = this->oled();
        oled->setFont(MENU_ITEM_FONT);
        oled->setCursor(0, MENU_ROW+position);
        oled->print(position==0?F(">"):F(" "));
        oled->print(name);
        if(value!=NULL) {
            oled->print(F(": "));
            oled->print(value);
        }
        oled->setFont(DEFAULT_FONT);
    }

    move(MENU_ROW_UART+position, 0);
    addstrF(position==0?F(">"):F(" "));
    addstrF(name);
    if(value!=NULL) {
        addstrF(F(": "));
        addstrF(value);
    }
}

void UI::printMenuItem(int8_t position, String name, String value) {
    SSD1306AsciiWire *oled = this->oled();
    oled->setCursor(0, MENU_ROW+position);
    oled->print(position==0?F(">"):F(" "));oled->print(name);
    if(value.length()>0) {
        oled->print(F(": "));oled->print(value);
    }
}