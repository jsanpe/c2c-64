#include "encoder.h"

byte Encoder::setOutputColorSpace(bool component) {
  this->componentOutput=component;
  return send_i2c(ENCODER_ADDR, 0x02, this->componentOutput?0x20:0x10);
}

byte Encoder::setInterlaced(bool interlaced){
  byte ret = 0;
  this->interlaced=interlaced;
  if (this->interlaced) {
    // Disable SD progressive mode
    ret += send_i2c(ENCODER_ADDR, 0x88, 0x00);
  } else {
    // Enable SD progressive mode
    ret += send_i2c(ENCODER_ADDR, 0x88, 0x02);
  }
  return ret;
}

byte Encoder::setTestMode(bool testMode){
  byte ret = 0;
  this->testMode=testMode;
  if(testMode) {
    ret += send_i2c(ENCODER_ADDR, 0x84, 0x40); // Enable color bars
    ret += send_i2c(ENCODER_ADDR, 0x87, 0x00);
    ret += send_i2c(ENCODER_ADDR, 0x88, 0x00);
  } else {
    ret += send_i2c(ENCODER_ADDR, 0x84, 0x06); //Enable SFL 
    ret += send_i2c(ENCODER_ADDR, 0x87, 0x20); //Automatic detection of input format
    ret += this->setInterlaced(this->interlaced);
  }
  if(pal) {
    byte pal_fsc[]={0xCB, 0x8A, 0x09, 0x2A};
    ret += send_i2c_string(ENCODER_ADDR  , 0x8C, pal_fsc, 4);
  } else {
    byte ntsc_fsc[]={0x1F, 0x7C, 0xF0, 0x21};
    ret += send_i2c_string(ENCODER_ADDR  , 0x8C, ntsc_fsc, 4);
  }

  return ret;
}

byte Encoder::setPal(bool pal){
  this->pal = pal;
  return setTestMode(this->testMode);
}


byte Encoder::reset() {
  byte ret = 0;
  ret += send_i2c(ENCODER_ADDR, 0x17, 0x02); // Reset
  delay(10);

  //Common intialization
  ret += send_i2c(ENCODER_ADDR, 0x00, 0x1C); //Dacs ON, PLL ON //0x1E -> PLL OFF
  ret += send_i2c(ENCODER_ADDR, 0x01, 0x00); //SD input mode
  ret += send_i2c(ENCODER_ADDR, 0x80, 0x11);
  // Pixel data valid. YPrPb out. SSAF PrPb filter enabled. Active video edge control enabled.
  ret += send_i2c(ENCODER_ADDR, 0x82, 0xC1);

  return ret;
}

byte Encoder::init() {
  byte ret = 0;
  if(this->testMode==false)
  {
    /*
    56 00 1C ; Power up DACs and PLL
    56 01 00 ; SD only mode
    56 80 10 ; SSAF Luma filter enabled, NTSC mode
    56 82 C9 ; Step control on, pixel data valid, pedestal on, PrPb SSAF on, YPrPb out.
    56 87 20 ; PAL/NTSC autodetect mode enabled
    56 88 00 ; 8 bit input enabled
    */
    ret += setOutputColorSpace(this->componentOutput);
    ret += setTestMode(this->testMode);
    ret += setInterlaced(this->interlaced);
  } else {
    ret += setTestMode(this->testMode);
  }

  return ret;
}
