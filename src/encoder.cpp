#include "encoder.h"
#include "config.h"

byte reset_encoder() {
  byte ret = 0;
  ret += send_i2c(ENCODER_ADDR, 0x17, 0x02); // Reset
  delay(10);
  return ret;
}

byte init_encoder(int test=0, byte interlaced_flag=0) {
  Serial.print("Init Encoder - test:");Serial.print(test, HEX);Serial.print("-Itl:");Serial.println(interlaced_flag);
  byte ret = 0;
  if(test==0)
  {
    /*
    56 00 1C ; Power up DACs and PLL
    56 01 00 ; SD only mode
    56 80 10 ; SSAF Luma filter enabled, NTSC mode
    56 82 C9 ; Step control on, pixel data valid, pedestal on, PrPb SSAF on, YPrPb out.
    56 87 20 ; PAL/NTSC autodetect mode enabled
    56 88 00 ; 8 bit input enabled
    */
    //encode input coming from the 
    ret += send_i2c(ENCODER_ADDR, 0x00, 0x1E); //Dacs ON, PLL ON //0x1E -> PLL OFF
    ret += send_i2c(ENCODER_ADDR, 0x01, 0x00); // SD input mode
    #ifdef OUTPUT_RGB
    ret += send_i2c(ENCODER_ADDR, 0x02, 0x10); // SD input mode
    #endif

    //ret += send_i2c(ENCODER_ADDR, 0x10, 0x10); //Enable DAC autopower-down (based on cable detection
    //PAL standard. SSAF luma filter enabled. 1.3 MHz chroma filter enabled.
    ret += send_i2c(ENCODER_ADDR, 0x80, 0x11); //PAL
    // Pixel data valid. YPrPb out. SSAF PrPb filter enabled. Active video edge control enabled.
    ret += send_i2c(ENCODER_ADDR, 0x82, 0xC1);
    //Enable SFL 
    ret += send_i2c(ENCODER_ADDR, 0x84, 0x06);
    //Automatic detection of input format
    ret += send_i2c(ENCODER_ADDR, 0x87, 0x20);

    if (interlaced_flag) {
      // Disable SD progressive mode
      ret += send_i2c(ENCODER_ADDR, 0x88, 0x00);
    } else {
      // Enable SD progressive mode
      ret += send_i2c(ENCODER_ADDR, 0x88, 0x02);
    }
  } else {
    //test -> generate pattern, bypass any inputs
    ret += send_i2c(ENCODER_ADDR, 0x00, 0x1E); 
    ret += send_i2c(ENCODER_ADDR, 0x01, 0x00); 
    ret += send_i2c(ENCODER_ADDR, 0x80, 0x11); 
    //ret += send_i2c(ENCODER_ADDR, 0x02, 0x24);  // Black bar pattern
    ret += send_i2c(ENCODER_ADDR, 0x82, 0xC1);  //YPbPr, SSAF filter, NO Pedestal
    ret += send_i2c(ENCODER_ADDR, 0x84, 0x40); // Enable color bars
    ret += send_i2c(ENCODER_ADDR, 0x87, 0x00);
    ret += send_i2c(ENCODER_ADDR, 0x88, 0x00);
    byte pal_fsc[]={0xCB, 0x8A, 0x09, 0x2A};
    ret += send_i2c_string(ENCODER_ADDR  , 0x8C, pal_fsc, 4);
  }

  return ret;
}
