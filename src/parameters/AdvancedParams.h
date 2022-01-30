#pragma once

#include "menus.h"
#include "i2c.h"

//#define DRIVE_STRENGTH_PARAM
#define ANTIALIAS_PARAM
#define IF_FILTER_PARAM
//#define DNR_PARAM
//#define SQUARE_PARAM

class AdvancedParameters: MenuProvider
{
  byte i2c_address;  

  class DriveStrengthHandler: public ArrayItemDelegate {
    byte i2c_address;
    //Drive Strength
    static const byte DriveStrengthRegister=0xF4;
    #ifdef DRIVE_STRENGTH_PARAM
    ArrayItemOO<3> drstrSyncItem;
    ArrayItemOO<3> drstrDataItem;
    ArrayItemOO<3> drstrClockItem;
    #endif
    byte drstrSyncValue = 1;
    byte drstrDataValue = 1;
    byte drstrClockValue = 1;

    public:
    DriveStrengthHandler(byte i2c_address){this->i2c_address=i2c_address;}
    byte getRegisterValue() {
        return ((this->drstrDataValue+1) << 4) | 
                ((this->drstrClockValue+1) << 2) | 
                (this->drstrSyncValue+1);
    }

    virtual void callback() {
        #ifdef DRIVE_STRENGTH_PARAM
        this->drstrSyncValue = this->drstrSyncItem.getRawValue();
        this->drstrDataValue = this->drstrDataItem.getRawValue();
        this->drstrClockValue = this->drstrClockItem.getRawValue();
        #endif
        send_i2c(i2c_address, DriveStrengthRegister, getRegisterValue());
    }
    
    void initMenu(MenuProvider *provider) {
        #ifdef DRIVE_STRENGTH_PARAM
        drstrSyncItem = ArrayItemOO<3>(provider, F("Sync Power"), drstrSyncValue, this);
        drstrSyncItem.addValue(F("Low"), F("Med"), F("High"));
        provider->addItem(&drstrSyncItem);

        drstrDataItem = ArrayItemOO<3>(provider, F("Data Power"), drstrDataValue, this);
        drstrDataItem.addValue(F("Low"), F("Med"), F("High"));
        provider->addItem(&drstrDataItem);

        drstrClockItem = ArrayItemOO<3>(provider, F("Clock Power"), drstrClockValue, this);
        drstrClockItem.addValue(F("Low"), F("Med"), F("High"));
        provider->addItem(&drstrClockItem);
        #endif
    }

  };
  DriveStrengthHandler drstrHandler;


  
  class AntialiasHandler: public ArrayItemDelegate {
    byte i2c_address;
    //Antialias
    static const byte AntialiasRegister=0xF3;
    byte antialiasValue = 1;
    #ifdef ANTIALIAS_PARAM
    ArrayItemOO<2> antialiasItem;
    #endif

    public:
    AntialiasHandler(byte i2c_address){this->i2c_address=i2c_address;}
    byte getRegisterValue() {
        return antialiasValue==1?0xF:0x8;
    }

    virtual void callback() {
        #ifdef ANTIALIAS_PARAM
        antialiasValue = antialiasItem.getRawValue();
        #endif
        send_i2c(i2c_address, AntialiasRegister, getRegisterValue());
    }

    void initMenu(MenuProvider *provider) {
        #ifdef ANTIALIAS_PARAM
        antialiasItem = ArrayItemOO<2>(provider, F("Antialias"), antialiasValue, this);
        antialiasItem.addValue(F("Off"), F("On"));
        provider->addItem(&antialiasItem);
        #endif
    }
  };
  AntialiasHandler antialiasHandler;


  class IfFilterHandler: public ArrayItemDelegate {
    byte i2c_address;
    //IF Filter
    static const byte IfFilterRegister=0xF8;
    #ifdef IF_FILTER_PARAM
    ArrayItemOO<4> ifFilterItem;
    #endif
    byte ifFilterValue = 0;

    public:
    IfFilterHandler(byte i2c_address){this->i2c_address=i2c_address;}
    byte getRegisterValue() {
        return (ifFilterValue==0?0x00:0x04)|ifFilterValue;
    }

    virtual void callback() {
        #ifdef IF_FILTER_PARAM
        ifFilterValue = ifFilterItem.getRawValue();
        #endif
        send_i2c(i2c_address, IfFilterRegister, getRegisterValue());
    }

    void initMenu(MenuProvider *provider) {
        #ifdef IF_FILTER_PARAM
        ifFilterItem = ArrayItemOO<4>(provider, F("IF Fitler"), ifFilterValue, this);
        ifFilterItem.addValue(F("Off"), F("Low"), F("Med"), F("High"));
        provider->addItem(&ifFilterItem);
        #endif
    }
  };
  IfFilterHandler ifFilterHandler;

  class DNRHandler: public ArrayItemDelegate {
    byte i2c_address;
    //DNR
    static const byte DNRRegister=0x4D;
    #ifdef DNR_PARAM
    ArrayItemOO<2> dnrItem;
    #endif
    byte dnrValue = 1;

    public:
    DNRHandler(byte i2c_address){this->i2c_address=i2c_address;}
    byte getRegisterValue() {
        return 0xEF & (dnrValue<<5);
    }

    virtual void callback() {
        #ifdef DNR_PARAM
        dnrValue = dnrItem.getRawValue();
        #endif
        send_i2c(i2c_address, DNRRegister, getRegisterValue());
    }

    void initMenu(MenuProvider *provider) {
        #ifdef DNR_PARAM
        dnrItem = ArrayItemOO<2>(provider, F("DNR"), dnrValue, this);
        dnrItem.addValue(F("Off"), F("On"));
        provider->addItem(&dnrItem);
        #endif
    }
  };
  DNRHandler dnrHandler;

  class SquarePixelHandler: public ArrayItemDelegate {
    byte i2c_address;
    //Square Pixel
    static const byte SquarePixelRegister=0x01;
    #ifdef SQUARE_PARAM
    ArrayItemOO<2> squarePixelItem;
    #endif
    byte squarePixelValue = 0;

    public:
    SquarePixelHandler(byte i2c_address){this->i2c_address=i2c_address;}
    byte getRegisterValue() {
        byte val = 0xC8 | (squarePixelValue << 2);
        return val;
    }

    virtual void callback() {
        #ifdef SQUARE_PARAM
        squarePixelValue = squarePixelItem.getRawValue();
        #endif
        send_i2c(i2c_address, SquarePixelRegister, getRegisterValue());
    }

    void initMenu(MenuProvider *provider) {
        #ifdef SQUARE_PARAM
        squarePixelItem = ArrayItemOO<2>(provider, F("Square Pixel"), squarePixelValue, this);
        squarePixelItem.addValue(F("Off"), F("On"));
        provider->addItem(&squarePixelItem);
        #endif
    }
  };
    
  SquarePixelHandler squarePixelHandler;

  public:
  AdvancedParameters(byte i2c_address):MenuProvider(F(">Advanced")),
    drstrHandler(i2c_address),
    antialiasHandler(i2c_address),
    ifFilterHandler(i2c_address),
    dnrHandler(i2c_address),
    squarePixelHandler(i2c_address)
  {
    this->i2c_address=i2c_address;
  }

  void callback() {
      drstrHandler.callback();
      antialiasHandler.callback();
      ifFilterHandler.callback();
      dnrHandler.callback();
      squarePixelHandler.callback();
  }

  virtual void initMenu(MenuProvider *provider) {
    drstrHandler.initMenu(this);
    antialiasHandler.initMenu(this);
    ifFilterHandler.initMenu(this);
    dnrHandler.initMenu(this);
    squarePixelHandler.initMenu(this);
    provider->addItem(this);
  }

  virtual MenuProvider *action() {
      return this;
  }
};