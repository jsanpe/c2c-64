#pragma once 

#include "decoder_status.h"
#include "menus.h"

class Decoder: public MenuProvider{
  public:
  Decoder():MenuProvider(F("Decoder")){}
  virtual byte setSource(bool cvbs)=0;
  virtual bool getSource()=0;

  virtual byte reset()=0;
  virtual byte init()=0;
  virtual DecoderStatus get_status()=0;
};