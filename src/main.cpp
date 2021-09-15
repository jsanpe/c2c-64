#include <Arduino.h>
#include <i2c.h>
#include "configmanager.h"
#include "encoder.h"
#include "mcurses.h"
#include "log.h"
#include "config.h"

#if (DECODER==ADV7280)
#include "decoder_7280Q32.h"
Decoder *decoder = new Decoder_7280Q32();
#elif (DECODER==ADV7180Q32)
#include "decoder_7180Q32.h"
Decoder *decoder = new Decoder_7180Q32();
#endif

void init_oled() {
  #ifdef OLED_ENABLED
    oled.begin(&Adafruit128x64, OLED_ADDR);
    oled.setFont(Adafruit5x7);
    oled.clear();
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
  pinMode(INPUT_SELECT , INPUT);
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
  _log("RD:"); _logln(err_dec);
  digitalWrite(LED_INPUT, err_dec==0?1:0);
  //Init Encoder
  byte err_enc = reset_encoder();
  _log("RE:"); _logln(err_enc);
  digitalWrite(LED_LOCK, err_enc==0?1:0);
  digitalWrite(LED_SIGNAL, 1);
  return err_dec+err_enc;
}

static byte err_init_adv = 0;

void change_input() {
  digitalWrite(LED_INPUT, mainStatus.input==CVBS?1:0);
  init_decoder(mainStatus.decoderFreeRun, mainStatus.input==CVBS);
}

void setup() {
  init_pins(); //setup GPIO
  hello_signal(); //blink leds with known pattern
  init_output(); //Init UART and I2C
  reset_adv(); //Reset Video decoder and encoder
  
  Serial.println("C2C-Initializing");
  err_init_adv = init_adv();
  delay(500);
  if(err_init_adv==0) {
    digitalWrite(LED_SIGNAL,1);
    delay(500);
    clear_leds();
    mainStatus.input = SVIDEO;
    mainStatus.decoderFreeRun = 0;
    change_input();
    delay(100);
    initscr();
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

}

void loop() {
  static int last_input = 0;
  if(err_init_adv!=0){Serial.println("Will not loop b/c init error");delay(1000);return;}

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

  int input_select = digitalRead(INPUT_SELECT);
  if(last_input>0 && input_select == 0) {
      last_input = 0;
      mainStatus.input = mainStatus.input==SVIDEO?CVBS:SVIDEO;
      change_input();
      return;
  }
  last_input = input_select;
  
  delay(50);

  while(Serial.available()>0) {
    int userInput = Serial.read();
    char userInputChar = (char)userInput;
    _log(userInputChar);
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