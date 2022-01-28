#include "parameters/YShape.h"

const byte ShapeFilterRegister=0x17;

byte YShapeParameter::getRegisterValue() {
  byte lValue=0;
  if(yShapeFilterMode < 2) { //AUTO
      lValue = yShapeFilterMode;
  } else if (yShapeFilterMode == 2) { //PAL
      lValue = 0x14 + yShapePalMode;
  } else if (yShapeFilterMode == 3) { //NTSC
      lValue = 0x19 + yShapeNtscMode;
  } else { //SHVS
      lValue = 0x2 + yShapeSVHSMode;
  }

  byte cValue=chromaShapeMode<<5;
  return cValue|lValue;
}

void YShapeParameter::callback() {
  //update all values to reuse this callback function across all items provided
  yShapeFilterMode = yShapeItem.getRawValue();
  yShapePalMode    = yShapePalModeItem.getRawValue();
  yShapeNtscMode   = yShapeNtscModeItem.getRawValue();
  yShapeSVHSMode   = yShapeSVHSModeItem.getRawValue();
  chromaShapeMode  = chromaSharpnessItem.getRawValue();

  send_i2c(this->i2c_address, ShapeFilterRegister, getRegisterValue());
}

void YShapeParameter::initMenu(MenuProvider *provider) {
  yShapeItem = ArrayItemOO<5>(provider, F("Y Shape"), yShapeFilterMode, this);
  yShapeItem.addValue(F("Auto 1"), F("Auto 2"), F("PAL"), F("NTSC"), F("SVHS"));
  provider->addItem(&yShapeItem);

  yShapePalModeItem = ArrayItemOO<5>(provider, F(" | PAL"), yShapePalMode, this);
  yShapePalModeItem.addValue(F("NN1"), F("NN2"), F("NN3"), F("WN1"), F("WN2"));
  provider->addItem(&yShapePalModeItem);

  yShapeNtscModeItem = ArrayItemOO<6>(provider, F(" | NTSC"), yShapeNtscMode, this);
  yShapeNtscModeItem.addValue(F("NN1"), F("NN2"), F("NN3"), F("WN1"), F("WN2"), F("WN3"));
  provider->addItem(&yShapeNtscModeItem);

  yShapeSVHSModeItem = ArrayItemOO<18>(provider, F(" | SVHS"), yShapeSVHSMode, this);
  yShapeSVHSModeItem.addValue(
      F("1"), F("2"), F("3"), F("4"), F("5"),
      F("6"), F("7"), F("8"), F("9"), F("10"),
      F("11"), F("12"), F("13"), F("14"), F("15"),
      F("16"), F("17"), F("18"));
  provider->addItem(&yShapeSVHSModeItem);

  chromaSharpnessItem = ArrayItemOO<8>(provider, F("C Shape"), chromaShapeMode, this);
  chromaSharpnessItem.addValue(F("1.5Mhz"), F("2.17Mhz"),F("SH1"), F("SH2"),F("SH3"), F("SH4"),F("SH5"),F("Wideband"));
  provider->addItem(&chromaSharpnessItem);
}