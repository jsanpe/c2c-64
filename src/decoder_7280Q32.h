#include "decoder.h"

class Decoder_7280Q32: public Decoder {
  public:
  virtual byte i2c_address();
  virtual byte reset();
  virtual byte init(int test, bool cvbs=true);
  virtual DecoderStatus get_status();
  virtual Parameter *getParams();
  //void change_param(int id, uint8_t sign);
};
