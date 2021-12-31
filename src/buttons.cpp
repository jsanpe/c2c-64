#include "buttons.h"

#define WAITING_CONFIRMATION (0X80)
#define FOUND_CONFIRMATION (0X8)
#define BUTTON_MASK (0x7)

byte ButtonStateMachine::step(bool source, bool plus, bool minus)
{
    byte new_status = (minus?1:0)<<2 | (plus?1:0)<<1 | (source?1:0);
    if(this->button_status&WAITING_CONFIRMATION) {
        if((this->button_status&BUTTON_MASK)==new_status) {
            //Two steps on the same status!
            this->button_status = FOUND_CONFIRMATION|new_status;
            return this->button_status&BUTTON_MASK;
        } else {
            //Buttons changed, stay on WAITING or release if no more buttons are pushed
            if(new_status==0) {
                this->button_status = 0;
            } else {
                this->button_status = WAITING_CONFIRMATION|new_status;
            }
            return 0;
        }
    } else if(this->button_status&FOUND_CONFIRMATION) {
        //Require going to 0 before accepting new presses
        if (new_status==0) {this->button_status=0;}
    }
    else {
        this->button_status = new_status;
        if(this->button_status) this->button_status |= WAITING_CONFIRMATION;
        return 0;
    }
    return 0;
}
