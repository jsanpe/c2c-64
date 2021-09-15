class DecoderStatus {
  public:
  byte lock;
  byte standard;
  byte interlaced;
  byte raw[4];
  byte i2c_error;
  byte i2c_error_code;
  
  DecoderStatus() {
    for(int i=0; i<4;i++) this->raw[i]=0xff;
    this->i2c_error = this->i2c_error_code = 0;
  }

  virtual const char *get_standard_name() {
    switch(this->standard) {
      case 0: return "NTSC M/J";
      case 1: return "NTSC 4.43";
      case 2: return "PAL M";
      case 3: return "PAL 60";
      case 4: return "PAL B/G/H/I/D";
      case 5: return "SECAM";
      case 6: return "PAL Combination N";
      case 7: return "SECAM 525";
    }
    return "Unknown";
  }

  bool equals(DecoderStatus target) {
    for(int i=0;i<4;i++) if(this->raw[i] != target.raw[i]) return false;
    return true;
  }

  /*
  DecoderStatus get_decoder_status(Decoder *decoder) {
    byte status_bytes[4];
    int count = read_i2c(decoder->i2c_address(), 0x10, status_bytes, 4);
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
      count = read_i2c(decoder->i2c_address(), 0x08, status_bytes, 4);
      if (count!=4) {
        status.i2c_error = 2;
        status.i2c_error_code = count;
      }
    }
    if(status.i2c_error==0) {
      status.contrast = status_bytes[0];
      status.brightness = status_bytes[2];
      status.hue = status_bytes[3];
    }
    if(status.i2c_error==0) {
      count = read_i2c(decoder->i2c_address(), 0x17, status_bytes, 3);
      if(count!=3) {
        status.i2c_error = 3;
        status.i2c_error_code = count;
      }
    }
    if(status.i2c_error==0){
      status.sh_filter = status_bytes[0];
      status.sh_filter_2 = status_bytes[1];
      status.comb_filter = status_bytes[2];
    }
    #ifdef COMB_PAL
    if(status.i2c_error==0){
      count = read_i2c(decoder->i2c_address(), 0x39, &status.comb_pal, 1);
      if(count!=1) {
        status.i2c_error = 4;
        status.i2c_error_code = count;
      } 
    }
    #endif
    #ifdef CTI_DNR
    if(status.i2c_error==0){
      count = read_i2c(decoder->i2c_address(), 0x4D, &status.cti_dnr, 1);
      if(count!=1) {
        status.i2c_error = 5;
        status.i2c_error_code = count;
      } 
    }
    #endif
    #ifdef IF_FILT
    count=0;
    if(status.i2c_error==0){
      count += read_i2c(decoder->i2c_address(), 0xF8, &status.iffilt, 1);
      count += read_i2c(decoder->i2c_address(), 0xFB, &status.peak, 1);
      count += read_i2c(decoder->i2c_address(), 0xFC, &status.thr, 1);
      if(count!=3) {
        status.i2c_error = 6;
        status.i2c_error_code = count;
      } 
    }
    #endif

    return status;
  }*/
};