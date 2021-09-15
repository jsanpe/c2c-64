#include <Arduino.h>
#include <Wire.h>

byte send_i2c_string(byte device, byte reg, byte *vals, int size);

byte send_i2c(byte device, byte reg, byte val);

int read_i2c(byte device, byte reg, byte* vals, byte size);