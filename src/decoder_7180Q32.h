#include "decoder.h"
#include "menus.h"

class Decoder_7180Q32: public Decoder {
  bool cvbs = true;
  byte lumaSharpness = 0;
  byte manualLuma = 0;
  byte chromaSharpness = 0;
  bool antialias = true;
  byte driveStrength = 0x15;
  byte autodetectMode = 0;
  byte squarePixelMode = 0;
  byte combFilterMode = 1;
  byte dnrMode = 1;
  byte ifMode = 0;

  BinaryMenuItem aaliasItem;
  TrinaryMenuItem drstrDataItem;
  TrinaryMenuItem drstrClockItem;
  TrinaryMenuItem autoYShapeItem;
  ArrayItem<29> manYshapeItem;
  ArrayItem<8> chromaSharpnessItem;
  ArrayItem<4> autodetectItem;
  BinaryMenuItem squarePixelItem;
  ArrayItem<4> combFilterItem;
  BinaryMenuItem dnrModeItem;
  ArrayItem<4> ifFilterItem;

  public:
  virtual byte i2c_address();
  virtual byte reset();
  virtual byte init();
  virtual DecoderStatus get_status();
  virtual byte setSource(bool cvbs);
  virtual bool getSource(){return this->cvbs;}
  
  void setIfFilterMode(byte value);
  void setDnrMode(byte value);
  void setCombFilterMode(byte value);
  void setAutodetectMode(byte value);
  void setSquarePixelMode(byte square);
  byte setLumaSharpness(byte sharp);
  byte getLumaSharpness(){return this->lumaSharpness;}
  byte setManualLuma(byte manualLuma){this->manualLuma=manualLuma;}
  byte getManualLuma(){return this->manualLuma;}
  byte setChromaSharpness(byte sharp);
  byte getChromaSharpness(){return this->chromaSharpness;}
  virtual bool getAntialias(){return this->antialias;}
  virtual byte setAntialias(bool antialias);
  byte setDriveStrength(byte value);
  byte getDriveStrength();
  virtual MenuProvider *action();

  virtual void initMenu(MenuProvider *root);
  fakestr getName(){return F("Decoder");}
};
