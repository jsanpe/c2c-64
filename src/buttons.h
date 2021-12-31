#pragma once
#include "Arduino.h"

#define SOURCE_BUTTON_PRESSED (0x1)
#define PLUS_BUTTON_PRESSED (0x2)
#define MINUS_BUTTON_PRESSED (0x4)
#define MENU_BUTTON_PRESSED (0x6)

class ButtonStateMachine{
    byte button_status = 0x0;
    public:
    byte step(bool source, bool plus, bool minus);
};