#include "i2c.h"

#define ENCODER_ADDR (0x54>>1)

byte reset_encoder();
byte init_encoder(int test=0, byte interlaced_flag=0);