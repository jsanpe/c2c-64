#pragma once

#include <Arduino.h>
#include <i2c.h>
#include "encoder.h"
#include "mcurses.h"
#include "config.h"
#include "menus.h"
#include "decoder.h"

typedef enum {CVBS, SVIDEO} input_t;
//void print_parameter_oled(Parameter *p, byte row, bool main=false, bool exit=false);
class BoardStatus {
  public:
  bool cvbs;
  bool pal;
  bool ntsc;
  bool secam;
  bool locked;
  bool interlaced;
  bool hz60;
};

//#define TEST_PATTERN_MENU_ITEM

class C2CBoard: public MenuProvider, ArrayItemDelegate {
  bool decoderFreeRun = false;
  bool encoderFreeRun = false;
  input_t input = CVBS;
  bool outputComponent = true;
  Decoder *decoder = NULL;
  Encoder *encoder = NULL;
  DecoderStatus decoderStatus;
  int i2c_error_count = 0;

  uint16_t initAdv();

  ArrayItemOO<2> outputColorSpaceItem;
  #ifdef TEST_PATTERN_MENU_ITEM
  BinaryMenuItem encoderFreeRunItem;
  #endif

  public:
  C2CBoard();
  Decoder *getDecoder();
  Encoder *getEncoder();


  uint16_t initVideoPipeline();
  void initMenu();

  input_t getSource(){return this->input;}
  void setSource(input_t input); 

  bool checkStatusChange();
  BoardStatus getCurrentStatus();
  void toggleInput();

  virtual void callback();
  
  //Uninteresting functions
  void disableFreeRun();
  bool isDecoderFreeRun();
  void setDecoderFreeRun(bool active);
  bool isEncoderFreeRun();
  void setEncoderFreeRun(bool active);

};