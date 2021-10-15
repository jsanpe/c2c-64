#include "config.h"
#if (DECODER==ADV7280)

#include "decoder_7280Q32.h"
#include "i2c.h"

#define COMB_PAL
#define COMB_NTSC
#define CTI_DNR

/*typedef enum {
  DEC_OUTPUT_RANGE, DEC_OUTPUT_SFL, DEC_OUTPUT_ITU34,
  DEC_CONTRAST, DEC_BRIGHTNESS, DEC_HUE, DEC_ANTIALIAS,
  DEC_Y_SHAPING_COMP, DEC_C_SHAPING_COMP, DEC_Y_SHAPING_SVIDEO, DEC_Y_SHAPING_SVIDEO_AUTO,
  DEC_COMB_FILTER_PAL, DEC_COMB_FILTER_NTSC,
  DEC_COMB_PAL_LUMA_MODE, DEC_COMB_PAL_CHROMA_MODE, DEC_COMB_PAL_CHROMA_TAPS,
  DEC_COMB_NTSC_LUMA_MODE, DEC_COMB_NTSC_CHROMA_MODE, DEC_COMB_NTSC_CHROMA_TAPS,
  DEC_CTI_DNR_MIX, DEC_IF_FILTER} DecParam;*/

Parameter decoder_7280Q32_params[]= {
    Parameter("OutRange", 0x04, 0x01),
    Parameter("SFL", 0x04, 0x02),
    Parameter("BT.656-3/4", 0x04, 0x80),
    Parameter("Contrast", 0x08, 0xFF),  //0x08
    Parameter("Brightness", 0x0A, 0xFF), //0x0A
    Parameter("Hue", 0x0B, 0xFF), //0x0B
    Parameter("YSFM", 0x17, 0x1F), //0x17
    Parameter("CSFM", 0x17, 0b11100000), //0x17
    Parameter("WYSFM", 0x18, 0x1F), //0x18
    Parameter("AUTO_WYSFM", 0x18, 0x80), //0x18
    Parameter("SFSEL_PAL", 0x19, 0x3), 
    Parameter("SFSEL_NTSC", 0x19, 0xC),

    #ifdef COMB_NTSC
    Parameter("NTSC Comb-L Mode", 0x38, 0x7), 
    Parameter("NTSC Comb-C Mode", 0x38, 0b00111000), 
    Parameter("NTSC Comb-C Taps", 0x38, 0b11000000), 
    #endif

    #ifdef COMB_PAL
    Parameter("PAL Comb-L Mode", 0x39, 0x7), 
    Parameter("PAL Comb-C Mode", 0x39, 0b00111000), 
    Parameter("PAL Comb-C Taps", 0x39, 0b11000000), 
    #endif

    #ifdef CTI_DNR
    Parameter("DNR Blend", 0x4D, 0b00001100),
    Parameter("IF Filter", 0xF8, 0b00000111),
    #endif

    Parameter("Antialias ON/OFF", 0xF3, 0x10), //0x0F3
    Parameter("Terminator", 0x00, 0x00, PARAM_TERMINATOR)
  };

Parameter *Decoder_7280Q32::getParams() {
  return decoder_7280Q32_params;
}

#define DECODER_ADDR this->i2c_address()
byte Decoder_7280Q32::i2c_address() {
  return  0x40>>1;
}

byte Decoder_7280Q32::reset() {
  byte ret = 0;
  send_i2c(DECODER_ADDR, 0x0F, 0x80); //Software reset
  delay(100);
  ret += send_i2c(DECODER_ADDR, 0x0F, 0x00); //Power on
  return ret;
}




byte Decoder_7280Q32::init(int test, bool cvbs) {
  Serial.print("Init Decoder - test:");Serial.print(test, HEX);Serial.print("-CVBS:");Serial.println(cvbs);
  this->reset();
  byte ret = 0;
  if(test==0)
  {
    //Select input signal and setup muxes
    if(cvbs) {
      Serial.println("Init for CVBS");
      ret += send_i2c(DECODER_ADDR, 0x52, 0xCD); //IBIAS
      ret += send_i2c(DECODER_ADDR, 0x00, 0x00); //CVBS
    } else {
      Serial.println("Init for SVIDEO");
      ret += send_i2c(DECODER_ADDR, 0x52, 0xCD); //IBIAS
      ret += send_i2c(DECODER_ADDR, 0x00, 0x09); //CVBS
      ret += send_i2c(DECODER_ADDR, 0x58, 0x04);
    }

    ret += send_i2c(DECODER_ADDR, 0x0E, 0x80);
    ret += send_i2c(DECODER_ADDR, 0x9C, 0x00);
    ret += send_i2c(DECODER_ADDR, 0x9C, 0xFF);
    ret += send_i2c(DECODER_ADDR, 0x0E, 0x00);

    byte buffer_adi[] = {0x51, 0x51, 0x68};
    ret += send_i2c_string(DECODER_ADDR, 0x80, buffer_adi, 3); //MWE Enable Manual Window, Colour Kill Threshold to 2

    //ret += send_i2c(DECODER_ADDR, 0x0C, 0x36); //Disable freerun    

    ret += send_i2c(DECODER_ADDR, 0x31, 0x02);

    ret += send_i2c(DECODER_ADDR, 0x17, 0x41); //SH Filter
    ret += send_i2c(DECODER_ADDR, 0x03, 0x0C); //Enable Pixel & Sync output drivers
    ret += send_i2c(DECODER_ADDR, 0x04, 0x37);
    ret += send_i2c(DECODER_ADDR, 0x13, 0x00); //Enable ADV7280A for 28_63636MHz crystal
    ret += send_i2c(DECODER_ADDR, 0x1D, 0x40); //Enable LLC output driver [ADV7280A writes finished]
    
    //byte buffer2[] = {0xA2, 0x6A, 0xA0};
    //ret += send_i2c_string(DECODER_ADDR, 0x3D, buffer2, 3); //MWE Enable Manual Window, Colour Kill Threshold to 2

    
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

DecoderStatus Decoder_7280Q32::get_status() {
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

#endif