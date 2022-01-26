#pragma once

#include "menus.h"
#include "i2c.h"

class AdvancedParameters: MenuProvider
{
  byte i2c_address;

  //Antialias
  static const byte AntialiasRegister=0xF3;
  ArrayItemOO<2> antialiasItem;
  byte antialiasValue = 1;
  
  //Drive Strength
  static const byte DriveStrengthRegister=0xF4;
  ArrayItemOO<3> drstrSyncItem;
  byte drstrSyncValue = 1;
  ArrayItemOO<3> drstrDataItem;
  byte drstrDataValue = 1;
  ArrayItemOO<3> drstrClockItem;
  byte drstrClockValue = 1;

  //IF Filter
  static const byte IfFilterRegister=0xF8;
  ArrayItemOO<4> ifFilterItem;
  byte ifFilterValue = 0;

  //DNR
  static const byte DNRRegister=0x4D;
  ArrayItemOO<2> dnrItem;
  byte dnrValue = 1;

  //Square Pixel
  static const byte SquarePixelRegister=0x01;
  ArrayItemOO<2> squarePixelItem;
  byte squarePixelValue = 0;

  class DriveStrengthHandler: public ArrayItemDelegate {
    AdvancedParameters *parent;
    public:
    DriveStrengthHandler(AdvancedParameters *parent){this->parent=parent;}
    byte getRegisterValue() {
        return ((parent->drstrDataValue+1) << 4) | 
                ((parent->drstrClockValue+1) << 2) | 
                (parent->drstrSyncValue+1);
    }

    void callback() {
        parent->drstrSyncValue = parent->drstrSyncItem.getRawValue();
        parent->drstrDataValue = parent->drstrDataItem.getRawValue();
        parent->drstrClockValue = parent->drstrClockItem.getRawValue();

        send_i2c(parent->i2c_address, DriveStrengthRegister, getRegisterValue());
    }
  };

  class AntialiasHandler: public ArrayItemDelegate {
    AdvancedParameters *parent;
    public:
    AntialiasHandler(AdvancedParameters *parent){this->parent=parent;}
    byte getRegisterValue() {
        return parent->antialiasValue==1?0xF:0x8;
    }

    void callback() {
        parent->antialiasValue = parent->antialiasItem.getRawValue();
        send_i2c(parent->i2c_address, AntialiasRegister, getRegisterValue());
    }
  };

  class IfFilterHandler: public ArrayItemDelegate {
    AdvancedParameters *parent;
    public:
    IfFilterHandler(AdvancedParameters *parent){this->parent=parent;}
    byte getRegisterValue() {
        return (parent->ifFilterValue==0?0x00:0x04)|parent->ifFilterValue;
    }

    void callback() {
        parent->ifFilterValue = parent->ifFilterItem.getRawValue();
        send_i2c(parent->i2c_address, IfFilterRegister, getRegisterValue());
    }
  };

  class DNRHandler: public ArrayItemDelegate {
    AdvancedParameters *parent;
    public:
    DNRHandler(AdvancedParameters *parent){this->parent=parent;}
    byte getRegisterValue() {
        return 0xEF & (parent->dnrValue<<5);;
    }

    void callback() {
        parent->dnrValue = parent->dnrItem.getRawValue();
        send_i2c(parent->i2c_address, DNRRegister, getRegisterValue());
    }
  };

  class SquarePixelHandler: public ArrayItemDelegate {
    AdvancedParameters *parent;
    public:
    SquarePixelHandler(AdvancedParameters *parent){this->parent=parent;}
    byte getRegisterValue() {
        byte val = 0xC8 | (parent->squarePixelValue << 2);
        return val;
    }

    void callback() {
        parent->squarePixelValue = parent->squarePixelItem.getRawValue();
        send_i2c(parent->i2c_address, SquarePixelRegister, getRegisterValue());
    }
  };

  DriveStrengthHandler *drstrHandler;
  AntialiasHandler *antialiasHandler;
  IfFilterHandler *ifFilterHandler;
  DNRHandler *dnrHandler;
  SquarePixelHandler *squarePixelHandler;

  public:
  AdvancedParameters(byte i2c_address):MenuProvider(F("Advanced")){
    this->i2c_address=i2c_address;
    this->drstrHandler = new DriveStrengthHandler(this);
    this->antialiasHandler = new AntialiasHandler(this);
    this->ifFilterHandler = new IfFilterHandler(this);
    this->dnrHandler = new DNRHandler(this);
    this->squarePixelHandler = new SquarePixelHandler(this);
  }

  void callback() {
      drstrHandler->callback();
      antialiasHandler->callback();
      ifFilterHandler->callback();
      dnrHandler->callback();
      squarePixelHandler->callback();
  }

  void initMenu(MenuProvider *provider) {
    antialiasItem = ArrayItemOO<2>(this, F("Antialias"), antialiasValue, antialiasHandler);
    antialiasItem.addValue(F("Off"), F("On"));
    this->addItem(&antialiasItem);

    ifFilterItem = ArrayItemOO<4>(this, F("IF Fitler"), ifFilterValue, ifFilterHandler);
    ifFilterItem.addValue(F("Off"), F("Low"), F("Med"), F("High"));
    this->addItem(&ifFilterItem);

    dnrItem = ArrayItemOO<2>(this, F("DNR"), dnrValue, dnrHandler);
    dnrItem.addValue(F("Off"), F("On"));
    this->addItem(&dnrItem);

    drstrSyncItem = ArrayItemOO<3>(this, F("Data Power"), drstrSyncValue, drstrHandler);
    drstrSyncItem.addValue(F("Low"), F("Med"), F("High"));
    this->addItem(&drstrSyncItem);

    drstrDataItem = ArrayItemOO<3>(this, F("Data Power"), drstrDataValue, drstrHandler);
    drstrDataItem.addValue(F("Low"), F("Med"), F("High"));
    this->addItem(&drstrDataItem);

    drstrClockItem = ArrayItemOO<3>(this, F("Clock Power"), drstrClockValue, drstrHandler);
    drstrClockItem.addValue(F("Low"), F("Med"), F("High"));
    this->addItem(&drstrClockItem);

    squarePixelItem = ArrayItemOO<2>(this, F("Square Pixel"), squarePixelValue, squarePixelHandler);
    squarePixelItem.addValue(F("Off"), F("On"));
    this->addItem(&squarePixelItem);

    provider->addItem(this);
  }
};