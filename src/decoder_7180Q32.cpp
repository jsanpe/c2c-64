#include "config.h"
#if (DECODER==ADV7180Q32)

#include "decoder_7180Q32.h"
#include "i2c.h"

#define DECODER_ADDR (this->i2c_address())
byte Decoder_7180Q32::i2c_address() {
  return  _i2c_address;
}

Decoder_7180Q32::Decoder_7180Q32():
  autodetectItem(this, F("Format"), DefaultAutoDetectMode, this)
{
  autodetectItem.addValue(F("PAL/NTSC_J"), F("PAL/NTSC_M"), F("PAL_N/NTSC_J"), F("PAL_N/NTSC_M"));
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

  return ret;
}


byte Decoder_7180Q32::init() {
  byte ret = 0;

  ret += setSource(this->cvbs);

  ret += send_i2c(DECODER_ADDR, 0xC4, 0x80);
  ret += send_i2c(DECODER_ADDR, 0x04, 0x57);
  ret += send_i2c(DECODER_ADDR, 0x0C, 0x36); 
  ret += send_i2c(DECODER_ADDR, 0x31, 0x02);

  byte buffer1[] = {0x40, 0x90, 0xF0};
  ret += send_i2c_string(DECODER_ADDR, 0x17, buffer1, 3); //SH1
  
  byte buffer2[] = {0xA2, 0x6A, 0xA0};
  ret += send_i2c_string(DECODER_ADDR, 0x3D, buffer2, 3); //MWE Enable Manual Window, Colour Kill Threshold to 2
  ret += send_i2c(DECODER_ADDR, 0x0E, 0x80);
  ret += send_i2c(DECODER_ADDR, 0x55, 0x81);
  ret += send_i2c(DECODER_ADDR, 0x0E, 0x00);

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

void Decoder_7180Q32::initMenu(MenuProvider *root) {
  yShapeParam.initMenu(this);
  yShapeParam.callback();
  combFilterParam.initMenu(this);
  combFilterParam.callback();
  this->addItem(&autodetectItem);
  advancedParam.initMenu(this);
  advancedParam.callback();
}

void Decoder_7180Q32::callback() {
  setAutodetectMode(autodetectItem.getRawValue());
}
void Decoder_7180Q32::setAutodetectMode(byte value){
  this->autodetectMode = value;
  byte source = this->cvbs?0:0x6;
  send_i2c(DECODER_ADDR, 0x00, (value<<4)|source);
}

MenuProvider *Decoder_7180Q32::action() {
  return this;
}

#endif