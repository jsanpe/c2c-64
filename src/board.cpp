#include "board.h"

#if (DECODER==ADV7280)
#include "decoder_7280Q32.h"
#define DECODER_CLASS Decoder_7280Q32
#elif (DECODER==ADV7180Q32)
#include "decoder_7180Q32.h"
#define DECODER_CLASS Decoder_7180Q32
#endif

/*******  MENU ITEMS *****/
void outputTypeCallback(MenuProvider *board, byte value) {
  ((C2CBoard*)board)->getEncoder()->setOutputColorSpace(value==0);
}
void encoderFreeRunCallback(MenuProvider *board, byte value) {
  ((C2CBoard*)board)->setEncoderFreeRun(value==0);
}

/****** C2C Board ******/
C2CBoard::C2CBoard():MenuProvider(F("Menu")) {
  this->decoder = new DECODER_CLASS();
  this->encoder = new Encoder();
}

void C2CBoard::initMenu() {
  ArrayMenuItemFactory itemFactory = ArrayMenuItemFactory(this);
  byte value = this->outputComponent?0:1;
  outputColorSpaceItem = itemFactory.createDuplet(F("Output"), F("YPbPr"), F("RGB"), value, outputTypeCallback);
  this->addItem(&outputColorSpaceItem);

  encoderFreeRunItem = itemFactory.createDuplet(F("Test Pattern"), F("Off"), F("On"), value, encoderFreeRunCallback);
  value = this->isEncoderFreeRun()?1:0;
  this->addItem(&encoderFreeRunItem);
  
  this->getDecoder()->initMenu(this);
  this->addItem(this->getDecoder());  
  //MenuProvider *imageConfig = new MenuProvider(F("Image Config"));
  //imageConfig->addItem(new BrightnessItem(this));
  //imageConfig->setRoot(this);
  this->setRoot(NULL);
}

Decoder *C2CBoard::getDecoder(){
  return decoder;
}

Encoder *C2CBoard::getEncoder(){
  return encoder;
}

uint16_t C2CBoard::initAdv() {
  //Init Decoder
  byte err_dec = this->getDecoder()->reset();
  digitalWrite(LED_INPUT, err_dec==0?1:0);
  //Init Encoder
  byte err_enc = getEncoder()->reset();
  digitalWrite(LED_LOCK, err_enc==0?1:0);
  digitalWrite(LED_SIGNAL, 1);
  getDecoder()->init();
  getEncoder()->init();
  return (((uint16_t)err_dec)<<8) | err_enc;
}

uint16_t C2CBoard::initVideoPipeline() {
  //Reset ADVs
  digitalWrite(RESET_ADV, 1);
  delay(100);
  digitalWrite(RESET_ADV, 0);
  delay(100);
  digitalWrite(RESET_ADV, 1);
  delay(100);
  return this->initAdv();
}

bool C2CBoard::checkStatusChange() {
  bool changesFlag = false;
  DecoderStatus status = getDecoder()->get_status();
  if(status.i2c_error!=0) {
    this->i2c_error_count++;
    if ((i2c_error_count%20)==0) {
      Serial.print("Many i2c err: ");Serial.println(i2c_error_count);
    }
    return false;
  }
  i2c_error_count = 0; //reset after a successfull read of status

  if(!status.equals(this->decoderStatus)){
    decoderStatus = status;
    digitalWrite(LED_LOCK, status.lock?1:0); //Update lock led status
    //init encoder to account for decoder change
    //Serial.print("Encoder FR"); Serial.println(this->encoderFreeRun);
    if(decoderStatus.lock) {
      byte ret = getEncoder()->setInterlaced(decoderStatus.interlaced);
      ret += getEncoder()->setPal(decoderStatus.isPAL());
    }
    
    return true;
  } /*else if (userInputFlag) {
    userInputFlag = false;
    print_status();
  }*/
  return false;
}

void C2CBoard::toggleInput() {
  setSource(this->input==CVBS?SVIDEO:CVBS);
}

BoardStatus C2CBoard::getCurrentStatus() {
  DecoderStatus decStatus = this->decoderStatus;
  BoardStatus status;
  status.cvbs = input==CVBS;
  status.hz60 = decStatus.is50hz()==false;
  status.interlaced = decStatus.interlaced;
  status.locked = decStatus.lock;
  status.ntsc = decStatus.isNTSC();
  status.pal = decStatus.isPAL();
  status.secam = decStatus.isSECAM();
  return status;
}

void C2CBoard::setSource(input_t input){
  this->input = input;
  digitalWrite(LED_INPUT, this->input==CVBS?1:0);
  getDecoder()->setSource(this->input==CVBS);
}


//Helper functions

void C2CBoard::disableFreeRun() {
  this->decoderFreeRun=false;
  this->encoderFreeRun=false; 
}

bool C2CBoard::isDecoderFreeRun() {return this->decoderFreeRun;}
void C2CBoard::setDecoderFreeRun(bool active) {
  this->decoderFreeRun = active;
}

bool C2CBoard::isEncoderFreeRun() {return this->encoderFreeRun;}
void C2CBoard::setEncoderFreeRun(bool active) {
  this->encoderFreeRun = active;
  this->getEncoder()->setTestMode(active);
}

//static ConfigManager config = ConfigManager(getDecoder());




/*
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
    getUI()->oled()->clear();
    status_mid(mainStatus.input==CVBS, lastDecoderStatus.lock, lastDecoderStatus.interlaced, 
      lastDecoderStatus.isNTSC(), lastDecoderStatus.isPAL(), lastDecoderStatus.isSECAM(),
      lastDecoderStatus.hzs==60, 0);
  }

}

void print_parameter_oled(Parameter *p, byte row, bool main=false, bool exit=false) {
  //getUI()->oled()->clearRow(row);
  getUI()->oled()->print(main?F("> "):F("  "));
  if(exit) getUI()->oled()->print(F("Exit menu"));
  else getUI()->oled()->print(p->name());
  //move(starty, 30); print_byte(config.get_param_value(p));
}*/

/*
void loop() {
  static int last_input = 0;
  static int counter_health=0;

  counter_health++;
  if(err_init_adv!=0){Serial.println(F("Will not loop b/c init error"));delay(1000);return;}


  if(counter_health>20) {
    counter_health=0;
    status_health();
  }

  
  int minus = digitalRead(BTN_MINUS);
  int plus = digitalRead(BTN_MINUS);
  if(in_menu){
    if(in_option){
      if(minus==0){
        userInputFlag = true;
        Parameter *p = decoder->getParams()+param_index;
        config.dec(p);
      } else if (plus==0) {
        userInputFlag = true;
        Parameter *p = decoder->getParams()+param_index;
        config.inc(p);
      }
    } else {
      if(minus==0){
        userInputFlag = true;
        if(param_index>0) param_index--;
      } else if (plus==0) {
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

    if(minus==plus && plus==0) {
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
}*/