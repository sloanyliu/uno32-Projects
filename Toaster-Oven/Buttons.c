/*
 * File:   Buttons.c
 * Author: Sloan Liu
 *
 * Created on February 18, 2018, 3:48 PM
 */


#include "xc.h"
#include "Buttons.h"

/**
 * This function initializes the proper pins such that the buttons 1-4 may be used by modifying
 * the necessary bits in TRISD/TRISF. Only the bits necessary to enable the 1-4 buttons are
 * modified so that this library does not interfere with other libraries.
 */
void ButtonsInit(void)
{
    TRISD |= 0x00E0;
    TRISF |= 0x0002;
}

/**
 * This function checks the button states and 
 * returns any events that have occured since the last
 * call. In the case of the first call to ButtonsCheckEvents() after ButtonsInit(), the function
 * should assume that the buttons start in an off state with value 0. Therefore if no buttons are
 * pressed when ButtonsCheckEvents() is first called, BUTTONS_EVENT_NONE should be returned. The
 * events are listed in the ButtonEventFlags enum at the top of this file. This function should be
 * called repeatedly.
 *
 * This function also performs debouncing of the buttons. Every time ButtonsCheckEvents() is called,
 * all button values are saved, up to the 4th sample in the past, so 4 past samples and the present
 * values. A button event is triggered if the newest 4 samples are the same and different from the
 * oldest sample. For example, if button 1 was originally down, button 1 must appear to be up for 4
 * samples and the last BTN3 event was BUTTON_EVENT_3DOWN before a BUTTON_EVENT_1UP is triggered. 
 * This eliminates button bounce, where the button may quickly oscillate between the ON and OFF state
 * as it's being pressed or released.
 *
 * NOTE: This will not work properly without ButtonsInit() being called beforehand.
 * @return A bitwise-ORing of the constants in the ButtonEventFlags enum or BUTTON_EVENT_NONE if no
 *         event has occurred.
 */
uint8_t ButtonsCheckEvents(void)
{
    static uint8_t s1, s2, s3, s4, s5, result2; //static vars to save values
    uint8_t result = 0x00;
    
    //MASK THE s5 == 0x00's and s1 == 0x00's so that it's checking for one bit being 0 or 1 instead of ALL buttons being 0 or 1
    if((s5 != s4) && (s4 == s3) && (s3 == s2) && (s2 == s1)){
        if((!(s5 & 0x01)) && (s1 & 0x01)){
            result |= BUTTON_EVENT_1DOWN;
        }
        if((!(s5 & 0x02)) && (s1 & 0x02)){
            result |= BUTTON_EVENT_2DOWN;
        }
        if((!(s5 & 0x04)) && (s1 & 0x04)){
            result |= BUTTON_EVENT_3DOWN;
        }
        if((!(s5 & 0x08)) && (s1 & 0x08)){
            result |= BUTTON_EVENT_4DOWN;
        }
        if((s5 & 0x01) && (!(s1 & 0x01))){
            result |= BUTTON_EVENT_1UP;
        }
        if((s5 & 0x02) && (!(s1 & 0x02))){
            result |= BUTTON_EVENT_2UP;
        }
        if((s5 & 0x04) && (!(s1 & 0x04))){
            result |= BUTTON_EVENT_3UP;
        }
        if((s5 & 0x08) && (!(s1 & 0x08))){
            result |= BUTTON_EVENT_4UP;
        }
    }
    
    //checking for repeat events
    if(result == result2){
        result |= BUTTON_EVENT_NONE;
    }
    
    //saving the event
    result2 = result;
    
    //saving the samples
    s5 = s4;
    s4 = s3;
    s3 = s2;
    s2 = s1;
    s1 = BUTTON_STATES();
    
    return result;
}
