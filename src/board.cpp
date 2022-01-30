#include "board.h"

#if (DECODER==ADV7280)
#include "decoder_7280Q32.h"
#define DECODER_CLASS Decoder_7280Q32
#elif (DECODER==ADV7180Q32)
#include "decoder_7180Q32.h"
Decoder_7180Q32 _decoder;
#define DECODER_INSTANCE (&_decoder)
#endif

Encoder _encoder;
#define ENCODER_INSTANCE (&_encoder)

/*******  MENU ITEMS *****/
void encoderFreeRunCallback(MenuProvider *board, byte value) {
  ((C2CBoard*)board)->setEncoderFreeRun(value==0);
}

void C2CBoard::callback() {
  outputComponent = outputColorSpaceItem.getRawValue()==0;
  this->encoder->setOutputColorSpace(outputComponent);
}
/****** C2C Board ******/
C2CBoard::C2CBoard():MenuProvider(F("Menu")), 
  outputColorSpaceItem(this, F("Output"), 0, this)
{
  this->decoder = DECODER_INSTANCE;
  this->encoder = ENCODER_INSTANCE;
}

void C2CBoard::initMenu() {
  outputColorSpaceItem.addValue(F("YPbPr"), F("RGB"));
  this->addItem(&outputColorSpaceItem);

  #ifdef TEST_PATTERN_MENU_ITEM
  encoderFreeRunItem = itemFactory.createDuplet(F("Test Pattern"), F("Off"), F("On"), value, encoderFreeRunCallback);
  value = this->isEncoderFreeRun()?1:0;
  this->addItem(&encoderFreeRunItem);
  #endif
  
  this->getDecoder()->initMenu(this);
  this->addItem(this->getDecoder());  

  this->setRoot(NULL);
}

Decoder *C2CBoard::getDecoder(){
  return decoder;
}

Encoder *C2CBoard::getEncoder(){
  return encoder;
}

uint16_t C2CBoard::initAdv() {
  //Init Decoder
  byte err_dec = this->getDecoder()->reset();
  digitalWrite(LED_INPUT, err_dec==0?1:0);
  //Init Encoder
  byte err_enc = getEncoder()->reset();
  digitalWrite(LED_LOCK, err_enc==0?1:0);
  digitalWrite(LED_SIGNAL, 1);
  getDecoder()->init();
  getEncoder()->init();
  return (((uint16_t)err_dec)<<8) | err_enc;
}

uint16_t C2CBoard::initVideoPipeline() {
  //Reset ADVs
  digitalWrite(RESET_ADV, 1);
  delay(100);
  digitalWrite(RESET_ADV, 0);
  delay(100);
  digitalWrite(RESET_ADV, 1);
  delay(100);
  return this->initAdv();
}

bool C2CBoard::checkStatusChange() {
  bool changesFlag = false;
  DecoderStatus status = getDecoder()->get_status();
  if(status.i2c_error!=0) {
    this->i2c_error_count++;
    if ((i2c_error_count%20)==0) {
      Serial.print("Many i2c err: ");Serial.println(i2c_error_count);
    }
    return false;
  }
  i2c_error_count = 0; //reset after a successfull read of status

  if(!status.equals(this->decoderStatus)){
    decoderStatus = status;
    digitalWrite(LED_LOCK, status.lock?1:0); //Update lock led status
    //init encoder to account for decoder change
    if(decoderStatus.lock) {
      byte ret = getEncoder()->setInterlaced(decoderStatus.interlaced);
      ret += getEncoder()->setPal(decoderStatus.isPAL());
    }
    
    return true;
  } /*else if (userInputFlag) {
    userInputFlag = false;
    print_status();
  }*/
  return false;
}

void C2CBoard::toggleInput() {
  setSource(this->input==CVBS?SVIDEO:CVBS);
}

BoardStatus C2CBoard::getCurrentStatus() {
  DecoderStatus decStatus = this->decoderStatus;
  BoardStatus status;
  status.cvbs = input==CVBS;
  status.hz60 = decStatus.is50hz()==false;
  status.interlaced = decStatus.interlaced;
  status.locked = decStatus.lock;
  status.ntsc = decStatus.isNTSC();
  status.pal = decStatus.isPAL();
  status.secam = decStatus.isSECAM();
  return status;
}

void C2CBoard::setSource(input_t input){
  this->input = input;
  digitalWrite(LED_INPUT, this->input==CVBS?1:0);
  getDecoder()->setSource(this->input==CVBS);
}


//Helper functions

void C2CBoard::disableFreeRun() {
  this->decoderFreeRun=false;
  this->encoderFreeRun=false; 
}

bool C2CBoard::isDecoderFreeRun() {return this->decoderFreeRun;}
void C2CBoard::setDecoderFreeRun(bool active) {
  this->decoderFreeRun = active;
}

bool C2CBoard::isEncoderFreeRun() {return this->encoderFreeRun;}
void C2CBoard::setEncoderFreeRun(bool active) {
  this->encoderFreeRun = active;
  this->getEncoder()->setTestMode(active);
}
