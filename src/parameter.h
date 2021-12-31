#pragma once
#include <Arduino.h>
#include "i2c.h"

#define PARAM_ALWAYS 0
#define PARAM_INFO 1
#define PARAM_DEBUG 2
#define PARAM_NEVER 3
#define PARAM_TERMINATOR 0xFF

class Parameter;

class ParamProvider {
  public:
  virtual Parameter *getParams()=0;
  virtual byte i2c_address()=0;
  virtual byte update_params_cache()=0;
};

class Parameter {
  friend class ConfigManager;
  byte _level;
  const char *_name;
  byte _reg;
  byte _mask;
  byte cached;
  public:
  const char *name(){return this->_name;}
  byte level(){return this->_level;}
  Parameter(){}
  Parameter(const char *name, byte reg, byte mask, byte level=PARAM_INFO) {
    this->_name = name;
    this->_level = level;
    this->_reg = reg;
    this->_mask = mask;
    this->cached = 0;
  }

  void inc(ParamProvider *prov){
      if(prov) {
          modify(prov, +1);
      }
  }
  void dec(ParamProvider *prov){
      if(prov) {
          modify(prov, -1);
      }
  }
  byte modify(ParamProvider *prov, int sign) {
      byte value = this->cached;
      byte imask = ~(this->mask());
      byte s = (imask + 1)&(this->mask());
      if(imask == 0) s = 5;
      byte new_value = ((value+(sign<0?-s:s))&(this->mask())) + (value&imask);
      //update register using i2c
      byte ret = send_i2c(prov->i2c_address(), this->reg(), new_value);
      if (ret==0) {
          //update param_map
          this->cached = new_value;
      }
      return ret;
  }
  byte get_param_value(Parameter *p) {
      byte reg_val = this->cached;
      byte mask = p->mask();

      while((mask&0x1)==0) {
          reg_val = reg_val>>1;
          mask = mask>>1;
      }

      return reg_val&mask;
  }
  byte mask(){return this->_mask;}
  byte reg(){return this->_reg;}
  void setCached(byte val){this->cached=val;}
};
