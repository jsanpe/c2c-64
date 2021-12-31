#include <Arduino.h>
#include "menus.h"


MultiValueMenuItem::MultiValueMenuItem(fakestr name, byte levels, byte value=0) {
    this->name = name;
    this->value = value;
    this->levels = levels;
}

fakestr MultiValueMenuItem::getName(){
    return this->name;
}
fakestr MultiValueMenuItem::getValue(){
    return this->getValue(value);
}

void MultiValueMenuItem::setValue(byte value){
    this->value = value;
}

void MultiValueMenuItem::setLevels(byte levels) {this->levels=levels;}

byte MultiValueMenuItem::getRawValue(){return value;}

MenuProvider* MultiValueMenuItem::action(){
    this->value = (this->value+1)%this->levels;
    return changeEvent();
}
