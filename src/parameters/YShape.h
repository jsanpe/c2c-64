#pragma once

#include "menus.h"
#include "i2c.h"

class YShapeParameter: ArrayItemDelegate
{
  ArrayItemOO<5> yShapeItem;
  ArrayItemOO<2> yShapeAutoModeItem;
  ArrayItemOO<5> yShapePalModeItem;
  ArrayItemOO<6> yShapeNtscModeItem;
  ArrayItemOO<18> yShapeSVHSModeItem;
  ArrayItemOO<8> chromaSharpnessItem;

  byte yShapeFilterMode = 2; //0:Auto_1, 1:Auto_2, 2:PAL, 3:NTSC, 4:SVHS
  byte yShapePalMode = 4; 
  byte yShapeNtscMode = 4;
  byte yShapeSVHSMode = 17;
  byte chromaShapeMode = 0;

  byte i2c_address;

  public:
  YShapeParameter(byte i2c_address){this->i2c_address=i2c_address;}
  
  byte getRegisterValue();

  void callback();
  void initMenu(MenuProvider *provider);
};