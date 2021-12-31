#pragma once

#include <Arduino.h>

class DecoderStatus {
  public:
  byte lock;
  byte standard;
  byte interlaced;
  byte raw[4];
  byte i2c_error;
  byte i2c_error_code;
  byte hzs;
  
  DecoderStatus() {
    for(int i=0; i<4;i++) this->raw[i]=0xff;
    this->i2c_error = this->i2c_error_code = 0;
  }

  virtual const char *get_standard_name() {
    switch(this->standard) {
      case 0: return "NTSC M/J";
      case 1: return "NTSC 4.43";
      case 2: return "PAL M";
      case 3: return "PAL 60";
      case 4: return "PAL B/G/H/I/D";
      case 5: return "SECAM";
      case 6: return "PAL Combination N";
      case 7: return "SECAM 525";
    }
    return "Unknown";
  }

  bool isNTSC(){return (this->standard>=0)&&(this->standard<=1);}
  bool isPAL(){return (this->standard>=2)&&(this->standard!=5)&&(this->standard!=7);}
  bool isSECAM(){return (this->standard==5)&&(this->standard==7);}
  bool is50hz(){return false;}

  bool equals(DecoderStatus target) {
    if (this->lock!=target.lock) return false;
    if (this->lock) {
      if (this->interlaced!=target.interlaced) return false;
      if (this->standard!=target.standard) return false;
      if (this->hzs!=target.hzs) return false;
    }
    return true;
  }
};