#include "configmanager.h"
#include "decoder_status.h"

class Decoder: public ParamProvider{
  public:
  virtual byte reset()=0;
  virtual byte init(int test, bool cvbs=true)=0;
  virtual DecoderStatus get_status()=0;
};