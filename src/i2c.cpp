#include <Arduino.h>
#include <Wire.h>

byte send_i2c_string(byte device, byte reg, byte *vals, int size) {
  Wire.beginTransmission(device);
  Wire.write(reg);
  for (int i = 0; i < size; i++) {
    byte val = *vals++;            
    Wire.write(val);
  }
  return Wire.endTransmission();
}

byte send_i2c(byte device, byte reg, byte val) {
  return send_i2c_string(device, reg, &val, 1);
}

int read_i2c(byte device, byte reg, byte* vals, byte size) {
  int count = Wire.requestFrom(device, size, reg, 1, true);
  for(int i=0;i<count;i++) {
    vals[i] = Wire.read();
  }
  Wire.endTransmission();
  return count;
}
