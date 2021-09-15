#pragma once
#include "parameter.h"

class ParamProvider {
  public:
  virtual Parameter *getParams()=0;
  virtual byte i2c_address()=0;
};

class ConfigManager {
    ParamProvider *provider;
    byte param_map[256];

    byte modify(Parameter *p, int sign);
    public:
    ConfigManager(ParamProvider *p);
    byte update_map();
    byte get_param_value(Parameter *p);
    void inc(Parameter *p);
    void dec(Parameter *p);
};

