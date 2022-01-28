#include <Arduino.h>

#include "config.h"

#include "board.h"
#include "ui.h"
#include "buttons.h"

C2CBoard board = C2CBoard();
UI ui = UI();
static ButtonStateMachine m = ButtonStateMachine();
uint16_t err_init_adv = 0;

void init_pins() {
  pinMode(LED_INPUT , OUTPUT);
  pinMode(LED_LOCK , OUTPUT);
  pinMode(LED_SIGNAL , OUTPUT);
  pinMode(RESET_ADV , OUTPUT);
  pinMode(BTN_INPUT , INPUT);
  pinMode(BTN_MINUS, INPUT);
  pinMode(BTN_PLUS, INPUT);
}

void clear_leds() {
  digitalWrite(LED_INPUT, 0);
  digitalWrite(LED_LOCK, 0);  
  digitalWrite(LED_SIGNAL, 0);
}

void hello_signal() {
  digitalWrite(LED_INPUT, 1);
  digitalWrite(LED_LOCK, 0);  
  digitalWrite(LED_SIGNAL, 1);
  delay(500);
  clear_leds();
}

void init_output() {
  //Init i2c bus
  Wire.begin();
  Wire.setClock(100000L);
  //Init UART
  Serial.begin(SERIAL_PORT_SPEED);
}

void setup() {
  init_pins(); //setup GPIO
  init_output(); //Init UART and I2C
  ui.initUI();
  Serial.println("M0");
  ui.logo(); //Show splashcreen on available interfaces
  hello_signal(); //blink leds with known pattern
  Serial.println("M1");
  err_init_adv = board.initVideoPipeline(); //Reset Video decoder and encoder
  digitalWrite(LED_INPUT, 1);
  Serial.println("M2");
  delay(1000);
  if(err_init_adv==0) {
    digitalWrite(LED_SIGNAL,1); //Signal correct bootup sequence with a short blink
    delay(500);
    clear_leds(); //Initialization ok: clear all leds
    //Set default input configuration (TODO: Read from EEPROM)
    board.setSource(DEFAULT_SOURCE);
    board.setEncoderFreeRun(DEFAULT_FREERUN);
    Serial.println("M3");
    board.initMenu();
    Serial.println("M4");
    delay(100);
    initscr();
    Serial.println("M5");
    ui.clearUI();
    Serial.println("M6");
  } else {
    ui.info("Error init:", (err_init_adv&0xff00)>>8, err_init_adv&0xff);
  }
}

typedef enum {NORMAL, MENU} mainstatus_t;

void loop() {
  static mainstatus_t userMode = NORMAL; 
  static int healthCounter=0;
  static MenuProvider *menuPointer = 0;
  static bool refreshFlag = true;
  //Step 1: Update UI if needed
  if(err_init_adv!=0){delay(1000);return;}
  //Serial.println("L1");
  bool changed = refreshFlag || board.checkStatusChange();
  if(changed) {
    refreshFlag = false;
    ui.clearUI();
    BoardStatus s = board.getCurrentStatus();
    if(userMode!=MENU) {
      ui.status_mid(s.cvbs, s.locked, s.interlaced, s.ntsc, s.pal, s.secam, s.hz60, 0);
    } else {
      ui.status_top(s.cvbs, s.locked, s.interlaced, s.ntsc, s.pal, s.secam, s.hz60, 0);
      
      //print menu
      MenuItem *item = menuPointer->getVisibleItem();
      if(item->prev()) ui.printMenuItem(-1, item->prev()->getName(), item->prev()->getValue());
      ui.printMenuItem(0, item->getName(), item->getValue());
      if(item->next()) ui.printMenuItem(1, item->next()->getName(), item->next()->getValue());
    }
  }
  if((++healthCounter)%10==0) ui.health();
  delay(50);
  //Step 2: Detect user input
  bool source = digitalRead(BTN_INPUT)==0; //active low
  bool minus = digitalRead(BTN_MINUS)==0;
  bool plus = digitalRead(BTN_PLUS)==0;
  byte buttonStatus = m.step(source, plus, minus);
  if(buttonStatus==0) {
    char userInput = ui.getUserInput();
    buttonStatus = userInput;
  }

  if(userMode==MENU){
    MenuItem *item = menuPointer->getVisibleItem();
    switch(buttonStatus) {
      case 0: break;
      case SOURCE_BUTTON_PRESSED:
        menuPointer = item->action();
        refreshFlag=true;
        if(menuPointer==NULL){userMode=NORMAL;board.browseReset();}
        break;
      case MINUS_BUTTON_PRESSED:
        menuPointer->browseUp();
        refreshFlag=true;
        break;
      case PLUS_BUTTON_PRESSED:
        menuPointer->browseDown();
        refreshFlag=true;
        break;
    }

  } else {
    if(buttonStatus!=0) {
      switch(buttonStatus){
        case SOURCE_BUTTON_PRESSED:
          board.toggleInput();
          refreshFlag=true;
          break;
        case PLUS_BUTTON_PRESSED:
          //board.plus();
          break;
        case MINUS_BUTTON_PRESSED:
          //TODO: Change profile
          //board.minus();
          break;
        case MENU_BUTTON_PRESSED:
          userMode = MENU;
          menuPointer=&board;
          refreshFlag=true;
          break;
      }
    } 
  }
}
