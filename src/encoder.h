#include "i2c.h"

#define ENCODER_ADDR (0x54>>1)


class Encoder {
    bool componentOutput = true;
    bool interlaced = false;
    bool testMode = false;
    bool pal = true;
    public:
    byte reset();
    byte init();
    byte setOutputColorSpace(bool component);
    byte setInterlaced(bool interlaced);
    byte setTestMode(bool testMode);
    byte setPal(bool pal);
};