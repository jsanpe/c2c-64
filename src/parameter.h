#pragma once
#include <Arduino.h>

#define PARAM_ALWAYS 0
#define PARAM_INFO 1
#define PARAM_DEBUG 2
#define PARAM_NEVER 3
#define PARAM_TERMINATOR 0xFF

class Parameter {
  friend class ConfigManager;
  byte _level;
  const char *_name;
  byte _reg;
  byte _mask;
  public:
  const char *name(){return this->_name;}
  byte level(){return this->_level;}
  Parameter(){}
  Parameter(const char *name, byte reg, byte mask, byte level=PARAM_INFO) {
    this->_name = name;
    this->_level = level;
    this->_reg = reg;
    this->_mask = mask;
  }

  byte mask(){return this->_mask;}
  byte reg(){return this->_reg;}
};
