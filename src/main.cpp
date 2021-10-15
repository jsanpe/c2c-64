#include <Arduino.h>
#include <i2c.h>
#include "configmanager.h"
#include "encoder.h"
#include "mcurses.h"
#include "log.h"
#include "config.h"
#include "menus.h"

#if (DECODER==ADV7280)
#include "decoder_7280Q32.h"
Decoder *decoder = new Decoder_7280Q32();
#elif (DECODER==ADV7180Q32)
#include "decoder_7180Q32.h"
Decoder *decoder = new Decoder_7180Q32();
#endif

bool has_oled = false;
void init_oled() {
  #ifdef OLED_ENABLED
    SSD1306AsciiWire *oled = getOled();
    has_oled=true;
    oled->begin(&Adafruit128x64, OLED_ADDR);
    oled->displayRemap(true);
    oled->setFont(Adafruit5x7);
    oled->clear();
    info("Oled screen found!");
  #endif
}

void reset_adv() {
  //Reset ADVs
  digitalWrite(RESET_ADV, 1);
  delay(100);
  digitalWrite(RESET_ADV, 0);
  delay(100);
  digitalWrite(RESET_ADV, 1);
  delay(100);
}

typedef enum {CVBS, SVIDEO} input_t;
class MainStatus {
  public:
  bool decoderFreeRun = false;
  bool encoderFreeRun = false;
  input_t input = CVBS;
};

DecoderStatus lastDecoderStatus;
MainStatus mainStatus;
bool userInputFlag = false;
static int i2c_error_count = 0;
static int param_index = 0;
static bool in_menu = false;
static bool in_option = false;
static ConfigManager config = ConfigManager(decoder);


void Arduino_putchar(uint8_t c)
{
  Serial.write(c);
}

void init_decoder(int test, bool source) {
  decoder->init(test, mainStatus.input==CVBS);
  config.update_map();
  delay(2400);
}

void init_pins() {
  pinMode(LED_INPUT , OUTPUT);
  pinMode(LED_LOCK , OUTPUT);
  pinMode(LED_SIGNAL , OUTPUT);
  pinMode(RESET_ADV , OUTPUT);
  pinMode(BTN_INPUT , INPUT);
  pinMode(BTN_MINUS, INPUT);
  pinMode(BTN_PLUS, INPUT);
}

void clear_leds() {
  digitalWrite(LED_INPUT, 0);
  digitalWrite(LED_LOCK, 0);  
  digitalWrite(LED_SIGNAL, 0);
}

void hello_signal() {
  digitalWrite(LED_INPUT, 1);
  digitalWrite(LED_LOCK, 0);  
  digitalWrite(LED_SIGNAL, 1);
  delay(500);
  clear_leds();
}

void init_output() {
  //Init i2c bus
  Wire.begin();
  Wire.setClock(100000L);
  //Init UART
  Serial.begin(115200);
  init_oled();
  setFunction_putchar(Arduino_putchar);
}

byte init_adv() {
  //Init Decoder
  byte err_dec = decoder->reset();
  //_log("RD:"); _logln(err_dec);
  digitalWrite(LED_INPUT, err_dec==0?1:0);
  //Init Encoder
  byte err_enc = reset_encoder();
  //_log("RE:"); _logln(err_enc);
  digitalWrite(LED_LOCK, err_enc==0?1:0);
  digitalWrite(LED_SIGNAL, 1);
  return err_dec+err_enc;
}

static byte err_init_adv = 0;
void print_status();

void change_input() {
  digitalWrite(LED_INPUT, mainStatus.input==CVBS?1:0);
  print_status();
  init_decoder(mainStatus.decoderFreeRun, mainStatus.input==CVBS);
}

void setup() {
  init_pins(); //setup GPIO

  init_output(); //Init UART and I2C
  logo();

  hello_signal(); //blink leds with known pattern
  reset_adv(); //Reset Video decoder and encoder
  
  Serial.println("C2C-Initializing");
  err_init_adv = init_adv();
  delay(1500);
  if(err_init_adv==0) {
    digitalWrite(LED_SIGNAL,1);
    delay(500);
    getOled()->clear();
    clear_leds();
    mainStatus.input = SVIDEO;
    mainStatus.decoderFreeRun = 0;
    change_input();
    delay(100);
    initscr();
    getOled()->clear();
  }
}

void print_byte(byte b) {
  char map[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  addstr("0x");addch(map[(b>>4)&0xf]);addch(map[b&0xf]);
}

void print_menu() {
  addstr("1-Toggle Decoder FR    2-Toggle Encoder FR    3-Toggle input source    A-Up    Z-Down    + Increase    - Decrease");
}

void print_parameter(Parameter *p, bool main=false) {
  uint_fast8_t starty;
  uint_fast8_t startx;
  getyx (starty, startx);

  addch(main?'>':' ');addch(' ');addstr(p->name());
  move(starty, 30); print_byte(config.get_param_value(p));
}

void print_parameter_oled(Parameter *p, byte row, bool main=false, bool exit=false);

void print_status() {

  clear();
  //_log_clear();
  move(0,0);
  print_menu();
  if(lastDecoderStatus.i2c_error!=0) {
    move(1,0);
    addstr("Err: ");
    print_byte(lastDecoderStatus.i2c_error);addstr(" - ");print_byte(lastDecoderStatus.i2c_error_code);
  }
  mvaddstr(2,0,"Interlaced: ");
    attrset(lastDecoderStatus.interlaced?B_GREEN:B_RED);
    addch(lastDecoderStatus.interlaced?'Y':'N');attrset(B_BLACK);

  addstr(" Locked: ");
    attrset(lastDecoderStatus.lock?B_GREEN:B_RED);
    addch(lastDecoderStatus.lock?'Y':'N');attrset(B_BLACK);

  mvaddstr(4,0,"Standard: ");addstr(lastDecoderStatus.get_standard_name());
  //mvaddstr(5,0,"IDENT:");print_byte(lastDecoderStatus.raw[1]);
  mvaddstr(6,0,"Encoder FR: ");addstr(mainStatus.encoderFreeRun?"True":"False");

  Parameter *p = decoder->getParams()+param_index;
  move(8,0); if(param_index>1) print_parameter(p-2);
  move(9,0); if(param_index>0) print_parameter(p-1);
  move(10,0); print_parameter(p, true);
  if(p->level()!=PARAM_TERMINATOR) {
    if((p+1)->level()!= PARAM_TERMINATOR) {
      move(11,0); print_parameter(p+1);
      if((p+2)->level()!= PARAM_TERMINATOR) {
        move(12,0); print_parameter(p+2);
      }
    }
  }
  move(15,0);
  refresh();

  //Oled
  if(in_menu) {
    status_top(mainStatus.input==CVBS, lastDecoderStatus.lock, lastDecoderStatus.interlaced, 
      lastDecoderStatus.isNTSC(), lastDecoderStatus.isPAL(), lastDecoderStatus.isSECAM(),
      lastDecoderStatus.hzs==60);

    Parameter *p = decoder->getParams()+param_index;
    //if(param_index>1) print_parameter_oled(p-2, 2, false, false);
    if(param_index>0) print_parameter_oled(p-1, 2, false, false);
    print_parameter_oled(p, 3, true, p->level()==PARAM_TERMINATOR);
    if(p->level()!=PARAM_TERMINATOR) {
      if((p+1)->level()!= PARAM_TERMINATOR) {
        print_parameter_oled(p+1, 4, true, p->level()==PARAM_TERMINATOR);
        if((p+2)->level()!= PARAM_TERMINATOR) {
          print_parameter_oled(p+2, 5, true, p->level()==PARAM_TERMINATOR);
        }
      }
    }

  } else {
    getOled()->clear();
    status_mid(mainStatus.input==CVBS, lastDecoderStatus.lock, lastDecoderStatus.interlaced, 
      lastDecoderStatus.isNTSC(), lastDecoderStatus.isPAL(), lastDecoderStatus.isSECAM(),
      lastDecoderStatus.hzs==60, 0);
  }

}

void print_parameter_oled(Parameter *p, byte row, bool main=false, bool exit=false) {
  //getOled()->clearRow(row);
  getOled()->print(main?F("> "):F("  "));
  if(exit) getOled()->print(F("Exit menu"));
  else getOled()->print(p->name());
  //move(starty, 30); print_byte(config.get_param_value(p));
}

void loop() {
  static int last_input = 0;
  static int counter_health=0;

  counter_health++;
  if(err_init_adv!=0){Serial.println(F("Will not loop b/c init error"));delay(1000);return;}

  DecoderStatus status = decoder->get_status();
  if(status.i2c_error!=0) {
    i2c_error_count++;
    if ((i2c_error_count%20)==0) {
      Serial.print("Many i2c err: ");Serial.println(i2c_error_count);
    }
    return;
  }
  i2c_error_count = 0; //reset after a successfull read of status

  if(!status.equals(lastDecoderStatus)){
    lastDecoderStatus = status;
    print_status();
    digitalWrite(LED_LOCK, status.lock?1:0); //Update lock led status
    //init encoder to account for decoder change
    byte ret = init_encoder(mainStatus.encoderFreeRun, lastDecoderStatus.interlaced);
  } else if (userInputFlag) {
    userInputFlag = false;
    print_status();
  }
  if(counter_health>20) {
    counter_health=0;
    status_health();
  }

  
  int minus = digitalRead(BTN_MINUS);
  int plus = digitalRead(BTN_MINUS);
  if(in_menu){
    if(in_option){
      if(minus){
        userInputFlag = true;
        Parameter *p = decoder->getParams()+param_index;
        config.dec(p);
      } else if (plus) {
        userInputFlag = true;
        Parameter *p = decoder->getParams()+param_index;
        config.inc(p);
      }
    } else {
      if(minus){
        userInputFlag = true;
        if(param_index>0) param_index--;
      } else if (plus) {
        userInputFlag = true;
        if(decoder->getParams()[param_index].level()!=PARAM_TERMINATOR) param_index++;
      }
    }


  } else {
    int input_select = digitalRead(BTN_INPUT);
    if(last_input>0 && input_select == 0) {
        last_input = 0;
        mainStatus.input = mainStatus.input==SVIDEO?CVBS:SVIDEO;
        change_input();
        return;
    } else {
      last_input = input_select;
    }

    if(minus==plus && plus==1) {
      in_menu=true;
      return;
    }
  }

  delay(50);

  while(Serial.available()>0) {
    int userInput = Serial.read();
    char userInputChar = (char)userInput;

    if(userInput>=(byte)'1' && userInput<=(byte)'3') {
      userInputFlag = true;
      byte userInputOption = userInput - (byte)'0';
      //Serial.print("Option selected: ");Serial.println(userInputOption);
      switch(userInputOption) {
        case 1: 
          mainStatus.decoderFreeRun = mainStatus.decoderFreeRun==true?false:true;
          init_decoder(mainStatus.decoderFreeRun, true);
          break;
        case 2: 
          mainStatus.encoderFreeRun = mainStatus.encoderFreeRun==true?false:true;
          init_encoder(mainStatus.encoderFreeRun, lastDecoderStatus.interlaced);
          break;
        case 3: 
          mainStatus.input = mainStatus.input==CVBS?SVIDEO:CVBS;
          change_input();
          break;
      }
    } else if(userInput=='a' || userInput=='A') {
      userInputFlag = true;
      if(param_index>0) param_index--;
    } else if(userInput=='z' || userInput=='Z') {
      userInputFlag = true;
      if(decoder->getParams()[param_index].level()!=PARAM_TERMINATOR) param_index++;
    } else if(userInput=='+') {
      userInputFlag = true;
      Parameter *p = decoder->getParams()+param_index;
      config.inc(p);
    } else if(userInput=='-') {
      userInputFlag = true;
      Parameter *p = decoder->getParams()+param_index;
      config.dec(p);
    }
  }
}