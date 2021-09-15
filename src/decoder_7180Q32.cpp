#include "config.h"
#if (DECODER==ADV7180Q32)

#include "decoder_7180Q32.h"
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

Parameter decoder_7180Q32_params[]= {
    Parameter("OutRange", 0x04, 0x01),
    Parameter("SFL", 0x04, 0x02),
    Parameter("BT.656-3/4", 0x04, 0x80),
    Parameter("Contrast", 0x08, 0xFF),  //0x08
    Parameter("Brightness", 0x0A, 0xFF), //0x0A
    Parameter("Hue", 0x0B, 0xFF), //0x0B
    Parameter("Antialias ON/OFF", 0xF3, 0xF1), //0x0F3
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

    Parameter("Terminator", 0x00, 0x00, PARAM_TERMINATOR)
  };

Parameter *Decoder_7180Q32::getParams() {
  return decoder_7180Q32_params;
}

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

byte Decoder_7180Q32::init(int test, bool cvbs) {
  byte ret = 0;
  if(test==0)
  {
    //Select input signal and setup muxes
    if(cvbs) {
      Serial.println("Init CVBS");
      ret += send_i2c(DECODER_ADDR, 0x00, 0x00); //CVBS
      ret += send_i2c(DECODER_ADDR, 0x3A, 0x17); //Mux0 enable
      ret += send_i2c(DECODER_ADDR, 0xC3, 0x01); //Mux0->Ain1
      ret += send_i2c(DECODER_ADDR, 0xC4, 0x80); //
      ret += send_i2c(DECODER_ADDR, 0x27, 0x58); //Luma/Chroma delay
      ret += send_i2c(DECODER_ADDR, 0x58, 0x00);
    } else {
      Serial.println("Init SVIDEO");
      ret += send_i2c(DECODER_ADDR, 0x00, 0x06); //SVIDEO
      ret += send_i2c(DECODER_ADDR, 0x3A, 0x13); //Mux0 + Mux1 enable
      ret += send_i2c(DECODER_ADDR, 0xC3, 0x54); //Mux0->Ain1, Mux1->Ain2
      ret += send_i2c(DECODER_ADDR, 0xC4, 0x80); //
      ret += send_i2c(DECODER_ADDR, 0x27, 0x69); //Luma/Chroma delay
      ret += send_i2c(DECODER_ADDR, 0x58, 0x04);
    }

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