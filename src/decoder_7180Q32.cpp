#include "config.h"
#if (DECODER==ADV7180Q32)

#include "decoder_7180Q32.h"
#include "i2c.h"

#define DECODER_ADDR this->i2c_address()
byte Decoder_7180Q32::i2c_address() {
  return  0x40>>1;
}

byte Decoder_7180Q32::reset() {
  byte ret = 0;
  send_i2c(DECODER_ADDR, 0x0F, 0x80); //Software reset
  delay(100);
  ret += send_i2c(DECODER_ADDR, 0x0F, 0x00); //Power on
  return ret;
}

byte Decoder_7180Q32::setSource(bool cvbs) {
  this->cvbs = cvbs;
  byte ret = 0;
  byte value = this->autodetectMode<<4;
  //Select input signal and setup muxes
  if(this->cvbs) {
    ret += send_i2c(DECODER_ADDR, 0x00, value|0x00); //CVBS
    ret += send_i2c(DECODER_ADDR, 0x3A, 0x17); //Mux0 enable
    ret += send_i2c(DECODER_ADDR, 0xC3, 0x01); //Mux0->Ain1
    ret += send_i2c(DECODER_ADDR, 0x27, 0x58); //Luma/Chroma delay
    ret += send_i2c(DECODER_ADDR, 0x52, 0xD); //AFE
    ret += send_i2c(DECODER_ADDR, 0x58, 0x00);
  } else {
    ret += send_i2c(DECODER_ADDR, 0x00, value|0x06); //SVIDEO
    ret += send_i2c(DECODER_ADDR, 0x3A, 0x13); //Mux0 + Mux1 enable
    ret += send_i2c(DECODER_ADDR, 0xC3, 0x54); //Mux0->Ain1, Mux1->Ain2
    ret += send_i2c(DECODER_ADDR, 0x27, 0x69); //Luma/Chroma delay
    ret += send_i2c(DECODER_ADDR, 0x52, 0xB); //AFE
    ret += send_i2c(DECODER_ADDR, 0x58, 0x04);
  }
  setAntialias(this->antialias);
  return ret;
}

byte set_sharpness(byte lumaSharpness, byte manualSharpness, byte chromaSharpness, byte address) {
  if(lumaSharpness<2) {
    return send_i2c(address, 0x17, chromaSharpness|lumaSharpness);
  } else {
    return send_i2c(address, 0x17, chromaSharpness|manualSharpness);
  }
}

byte Decoder_7180Q32::setLumaSharpness(byte sharp) {
  this->lumaSharpness = sharp;
  byte ret = 0;
  set_sharpness(this->lumaSharpness, this->manualLuma, this->chromaSharpness, DECODER_ADDR);
  return ret;
}

byte Decoder_7180Q32::setChromaSharpness(byte sharp) {
  this->chromaSharpness = sharp;
  byte ret = 0;
  set_sharpness(this->lumaSharpness, this->manualLuma, this->chromaSharpness, DECODER_ADDR);
  return ret;
}

byte Decoder_7180Q32::setAntialias(bool antialias) {
  this->antialias = antialias;
  return send_i2c(DECODER_ADDR, 0xF3, antialias?0x0F:0x8);//antialias?0x9+(this->cvbs?0:2):0);
}

byte Decoder_7180Q32::init() {
  byte ret = 0;
  bool test = false;  
  if(!test)
  {
    ret += setSource(this->cvbs);

    ret += send_i2c(DECODER_ADDR, 0x18, 0x13); //Auto selection of WYFSM filter

    ret += send_i2c(DECODER_ADDR, 0xC4, 0x80);
    ret += send_i2c(DECODER_ADDR, 0x04, 0x57);
    ret += send_i2c(DECODER_ADDR, 0x0C, 0x36); //Disable freerun    
    ret += send_i2c(DECODER_ADDR, 0x31, 0x02);

    /*
    byte buffer1[] = {0x40, 0x90, 0xF0};
    ret += send_i2c_string(DECODER_ADDR, 0x17, buffer1, 3); //SH1
    */
    
    byte buffer2[] = {0xA2, 0x6A, 0xA0};
    ret += send_i2c_string(DECODER_ADDR, 0x3D, buffer2, 3); //MWE Enable Manual Window, Colour Kill Threshold to 2
    ret += send_i2c(DECODER_ADDR, 0x0E, 0x80);
    ret += send_i2c(DECODER_ADDR, 0x55, 0x81);
    ret += send_i2c(DECODER_ADDR, 0x0E, 0x00);
    
  }/* else {
    //test -> generate test pattern
    ret += send_i2c(DECODER_ADDR, 0x17, 0x41); //SH1
    ret += send_i2c(DECODER_ADDR, 0x03, 0x0c); 
    ret += send_i2c(DECODER_ADDR, 0x04, ext_outc);
    ret += send_i2c(DECODER_ADDR, 0x31, 0x0A);
    //ret += send_i2c(DECODER_ADDR, 0x1D, 0x40);
    //ret += send_i2c(DECODER_ADDR, 0xF4, 0x15); //output Power for clock signal
    send_i2c(DECODER_ADDR, 0X0D, 0x33);
    ret += send_i2c(DECODER_ADDR, 0x0c, 0x37); //Output blue screen for testing purposes
    ret += send_i2c(DECODER_ADDR, 0x0D, 0xC7);
    //ret += send_i2c(DECODER_ADDR, 0x8F, 0x00); //LLC at 27Mhz
  }*/
  return ret;
}

DecoderStatus Decoder_7180Q32::get_status() {
  byte status_bytes[4];
  int count = read_i2c(this->i2c_address(), 0x10, status_bytes, 4);
  DecoderStatus status;
  if (count!=4) {
    status.i2c_error = 1;
    status.i2c_error_code = count;
  } 
  if(status.i2c_error==0) {
    status.i2c_error = status.i2c_error_code = 0;
    status.lock = status_bytes[0] & 0x1;
    status.standard = (status_bytes[0] & 0x70) >> 4;
    status.interlaced = (status_bytes[3] & 0x40) >> 6;
    status.hzs = ((status_bytes[3] & 0x04) >> 2)==0?60:50;
    for(int i=0;i<4;i++) status.raw[i] = status_bytes[i];
  }

  //Read control registers
  if(status.i2c_error==0) {
    count = read_i2c(this->i2c_address(), 0x08, status_bytes, 4);
    if (count!=4) {
      status.i2c_error = 2;
      status.i2c_error_code = count;
    }
  }
  return status;
}

void dataDriverStrengthCallback(MenuProvider *dec, byte value) {
  byte val = ((value+1)&0x3)<<4;
  Decoder_7180Q32 *decoder = (Decoder_7180Q32*)dec;
  decoder->setDriveStrength((decoder->getDriveStrength()&0xCF)|val);
}

void clockDriverStrengthCallback(MenuProvider *dec, byte value) {
  byte val = ((value+1)&0x3)<<2;
  Decoder_7180Q32 *decoder = (Decoder_7180Q32*)dec;
  decoder->setDriveStrength((decoder->getDriveStrength()&0xF3)|val);
}

void yShapeCallback(MenuProvider *dec, byte value) {
  Decoder_7180Q32 *decoder = (Decoder_7180Q32*)dec;
  decoder->setLumaSharpness(value);
}

void yShapeManualCallback(MenuProvider *dec, byte value) {
  Decoder_7180Q32 *decoder = (Decoder_7180Q32*)dec;
  value+=2;
  decoder->setManualLuma(value);
  if(decoder->getLumaSharpness()==2) set_sharpness(2, value, decoder->getChromaSharpness(), decoder->i2c_address());
}

void cShapeCallback(MenuProvider *dec, byte value) {
  Decoder_7180Q32 *decoder = (Decoder_7180Q32*)dec;
  value=value<<5;
  decoder->setChromaSharpness(value);
}


void antialiasCallback(MenuProvider *dec, byte value) {
  ((Decoder_7180Q32*)dec)->setAntialias(value==1);
}

void autodetectModeCallback(MenuProvider *dec, byte value) {
  ((Decoder_7180Q32*)dec)->setAutodetectMode(value);
}

void squarePixelCallback(MenuProvider *dec, byte value) {
  ((Decoder_7180Q32*)dec)->setSquarePixelMode(value);
}

void combFilterCallback(MenuProvider *dec, byte value) {
  ((Decoder_7180Q32*)dec)->setCombFilterMode(value);
}

void dnrCallback(MenuProvider *dec, byte value) {
  ((Decoder_7180Q32*)dec)->setDnrMode(value);
}

void ifCallback(MenuProvider *dec, byte value) {
  ((Decoder_7180Q32*)dec)->setIfFilterMode(value);
}

void Decoder_7180Q32::initMenu(MenuProvider *root) {
  ArrayMenuItemFactory itemFactory = ArrayMenuItemFactory(this);
  byte value = this->autodetectMode;

  combFilterItem = ArrayItem<4>(this, F("Comb Filter"), value, combFilterCallback);
  combFilterItem.addValue(F("Narrow"), F("Medium"), F("Wide"), F("Widest"));
  this->addItem(&combFilterItem);

  value = this->dnrMode;
  dnrModeItem = itemFactory.createDuplet(F("DNR"), F("Off"), F("On"), 0, dnrCallback);
  this->addItem(&dnrModeItem);

  autodetectItem = ArrayItem<4>(this, F("Format"), value, autodetectModeCallback);
  autodetectItem.addValue(F("PAL/NTSC_J"), F("PAL/NTSC_M"), F("PAL_N/NTSC_J"), F("PAL_N/NTSC_M"));
  this->addItem(&autodetectItem);

  value = this->squarePixelMode;
  squarePixelItem = itemFactory.createDuplet(F("Square Pixel"), F("Off"), F("On"), 0, squarePixelCallback);
  this->addItem(&squarePixelItem);

  value = this->getAntialias()?1:0;
  aaliasItem = itemFactory.createDuplet(F("Antialias"), F("Off"), F("On"), 0, antialiasCallback);
  this->addItem(&aaliasItem);
  
  value = this->getLumaSharpness();
  autoYShapeItem = itemFactory.createTriplet(F("Y Shape"), F("Smooth"), F("Sharp"), F("Manual"), value, yShapeCallback);
  this->addItem(&autoYShapeItem);

  value = this->getManualLuma();
  manYshapeItem = ArrayItem<29>(this, F(" \\ Manual"), value, yShapeManualCallback);
  manYshapeItem.addValue(
    F("SVHS 1"), F("SVHS 2"), F("SVHS 3"), F("SVHS 4"), F("SVHS 5"),
    F("SVHS 6"), F("SVHS 7"), F("SVHS 8"), F("SVHS 9"), F("SVHS 10"),
    F("SVHS 11"), F("SVHS 12"), F("SVHS 13"), F("SVHS 14"), F("SVHS 15"),
    F("SVHS 16"), F("SVHS 17"), F("SVHS 18"), F("PAL NN1"), F("PAL NN2"),
    F("PAL NN3"), F("PAL WN1"), F("PAL WN2"), F("NTSC NN1"), F("NTSC NN2"),
    F("NTSC NN3"), F("NTSC WN1"), F("NTSC WN2"), F("NTSC WN3"));
  this->addItem(&manYshapeItem);

  value = this->getChromaSharpness();
  chromaSharpnessItem = ArrayItem<8>(this, F("C Shape"), value, cShapeCallback);
  chromaSharpnessItem.addValue(F("1.5Mhz"), F("2.17Mhz"),F("SH1"),  F("SH2"),F("SH3"), F("SH4"),F("SH5"),F("Wideband"));
  this->addItem(&chromaSharpnessItem);

  ifFilterItem = ArrayItem<4>(this, F("IF Fitler"), value, autodetectModeCallback);
  ifFilterItem.addValue(F("Off"), F("Low"), F("Med"), F("High"));
  this->addItem(&ifFilterItem);

  value = this->getDriveStrength();
  drstrDataItem = itemFactory.createTriplet(F("Data Power"), F("Low"), F("Med"), F("High"), ((value>>4)&0x3)-1, dataDriverStrengthCallback);
  this->addItem(&drstrDataItem);
  drstrClockItem = itemFactory.createTriplet(F("Clock Power"), F("Low"), F("Med"), F("High"), ((value>>2)&0x3)-1, clockDriverStrengthCallback);
  this->addItem(&drstrClockItem);

  /*MenuProvider *imageConfig = new MenuProvider(F("Image Config"));
  imageConfig->addItem(new BrightnessItem(this));
  imageConfig->setRoot(this);*/
}

void Decoder_7180Q32::setIfFilterMode(byte value){
  this->ifMode = value;
  byte val = value==0?0:0x4+value;
  return send_i2c(DECODER_ADDR, 0xF8, val);
}


void Decoder_7180Q32::setDnrMode(byte value){
  this->dnrMode = value;
  byte val = 0xEF & (value<<5);
  return send_i2c(DECODER_ADDR, 0x4D, val);
}

void Decoder_7180Q32::setCombFilterMode(byte value){
  this->combFilterMode = value;
  byte val = 0xF0 | (value << 2) | value;
  return send_i2c(DECODER_ADDR, 0x19, val);
}


void Decoder_7180Q32::setSquarePixelMode(byte value){
  this->squarePixelMode = value;
  byte val = 0xC8 | (value << 2);
  return send_i2c(DECODER_ADDR, 0x01, val);
}


void Decoder_7180Q32::setAutodetectMode(byte value){
  this->autodetectMode = value;
  byte source = this->cvbs?0:0x6;
  return send_i2c(DECODER_ADDR, 0x00, (value<<4)|source);
}

byte Decoder_7180Q32::setDriveStrength(byte value){
  this->driveStrength=value;
  return send_i2c(DECODER_ADDR, 0xF4, value);
}

byte Decoder_7180Q32::getDriveStrength(){return this->driveStrength;}

MenuProvider *Decoder_7180Q32::action() {
  return this;
}



#endif