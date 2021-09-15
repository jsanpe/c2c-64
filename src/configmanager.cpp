
#include "configmanager.h"
#include "i2c.h"
#include "mcurses.h"

/*Parameter *ConfigManager::search(int param_id) {
    Parameter *p = this->provider->getParams();
    for(;p->level()!=PARAM_TERMINATOR;p++) {
        if(p->id()==param_id) return p;
    }
    return 0;        
}*/
/*
void ConfigManager::inc(int param_id){
    Parameter *p = this->search(param_id);
    if(p) {
        modify(p, +1);
    }
}
void ConfigManager::dec(int param_id){
    Parameter *p = this->search(param_id);
    if(p) {
        modify(p, -1);
    }
}*/

void ConfigManager::inc(Parameter *p){
    if(p) {
        modify(p, +1);
    }
}
void ConfigManager::dec(Parameter *p){
    if(p) {
        modify(p, -1);
    }
}
byte ConfigManager::modify(Parameter *p, int sign) {
    byte value = this->param_map[p->reg()];
    byte imask = ~(p->mask());
    byte s = (imask + 1)&(p->mask());
    if(imask == 0) s = 5;
    byte new_value = ((value+(sign<0?-s:s))&(p->mask())) + (value&imask);
    //update register using i2c
    byte ret = send_i2c(this->provider->i2c_address(), p->reg(), new_value);
    if (ret==0) {
        //update param_map
        this->param_map[p->reg()] = new_value;
    }
    return ret;
}

ConfigManager::ConfigManager(ParamProvider *p)
{
    for(int i=0;i<256;i++) this->param_map[i]=0;
    this->provider = p;
}


byte ConfigManager::update_map() {
    byte value;
    byte ret = 0;
    Parameter *p = this->provider->getParams();
    for(;p->level()!=PARAM_TERMINATOR;p++) {
        ret = read_i2c(this->provider->i2c_address(), p->reg(), &value, 1);
        if (ret == 1) this->param_map[p->reg()] = value;
        else {
            Serial.print("Error reading register: ");Serial.println(p->reg());
        }
    }
    return ret;
}

void print_byte(byte b);
byte ConfigManager::get_param_value(Parameter *p) {
    byte reg_val = this->param_map[p->reg()];
    byte mask = p->mask();

    while((mask&0x1)==0) {
        reg_val = reg_val>>1;
        mask = mask>>1;
    }

    return reg_val&mask;
}