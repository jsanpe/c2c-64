#pragma once

#include "menus.h"
#include "i2c.h"

class CombFilterParameter: ArrayItemDelegate
{
  static const byte CombFilterRegister=0x19;
  ArrayItemOO<4> combFilterItem;
  byte combFilterMode = 0; 
  byte i2c_address;

  public:
  CombFilterParameter(byte i2c_address){this->i2c_address=i2c_address;}
  
  byte getRegisterValue() {
      return 0xF0 | (combFilterMode << 2) | combFilterMode;
  }

  void callback() {
    this->combFilterMode = combFilterItem.getRawValue();
    send_i2c(i2c_address, CombFilterRegister, getRegisterValue());
  }
  void initMenu(MenuProvider *provider) {
    combFilterItem = ArrayItemOO<4>(provider, F("Comb Filter"), combFilterMode, this);
    combFilterItem.addValue(F("Narrow"), F("Medium"), F("Wide"), F("Widest"));
    provider->addItem(&combFilterItem);
  }
};