#pragma once 

#include "decoder.h"
#include "menus.h"
#include "parameters/YShape.h"
#include "parameters/CombFilter.h"
#include "parameters/AdvancedParams.h"

class Decoder_7180Q32: public Decoder, ArrayItemDelegate {
  static const byte _i2c_address = 0x40>>1;
  static const byte DefaultAutoDetectMode = 0;
  bool cvbs = true;
  byte autodetectMode = DefaultAutoDetectMode;
  
  YShapeParameter yShapeParam = YShapeParameter(_i2c_address);
  CombFilterParameter combFilterParam = CombFilterParameter(_i2c_address);
  AdvancedParameters advancedParam = AdvancedParameters(_i2c_address);
  ArrayItemOO<4> autodetectItem;
  
  public:
  Decoder_7180Q32();
  virtual byte i2c_address();
  virtual byte reset();
  virtual byte init();
  virtual DecoderStatus get_status();
  virtual byte setSource(bool cvbs);
  virtual bool getSource(){return this->cvbs;}
  
  void setAutodetectMode(byte value);

  virtual MenuProvider *action();

  virtual void initMenu(MenuProvider *root);
  virtual void callback();
  fakestr getName(){return F(">Decoder");}
};
