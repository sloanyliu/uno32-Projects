// **** Include libraries here ****
// Standard C libraries

//The heap size I used was: 1024 * 3 = 3072

//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>
#include <string.h>
#include "Tree.h"
#include "Morse.h"
#include "Buttons.h"
#include "Oled.h"
#include "OledDriver.h"

// User libraries


// **** Set any macros or preprocessor directives here ****
// Specify a bit mask for setting/clearing the pin corresponding to BTN4. Should only be used when
// unit testing the Morse event checker.
#define BUTTON4_STATE_FLAG (1 << 7)

// **** Declare any data types here ****

// **** Define any module-level, global, or external variables here ****
static MorseEvent yay; //the morseEvent 
static uint8_t getit; //return the output of MorseDecode()
static char bottom[64] = "\n"; //the string to hold the decoded characters
static int x = -6; //starting pixel number for x, used in OledDrawChar(x,y,char);

// **** Declare any function prototypes here ****
static void morseLineClear(void); //clears the top line
static void addMorseChar(char letter); //updates the top line
static void addLetter(char letter); //updates the bottom line

int main()
{
    BOARD_Init();

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, BOARD_GetPBClock() / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

/******************************************************************************
 * Your code goes in between this comment and the following one with asterisks.
 *****************************************************************************/
    MorseInit(); //make the morse tree
    OledInit(); //start the OLED
   
    //Basically a state machine
    while(1){
        if(yay == MORSE_EVENT_NONE){
            ; //do nothing when its MORSE_EVENT_NONE
        }else if(yay == MORSE_EVENT_DOT){ //print dot, decode dot
            getit = MorseDecode(MORSE_CHAR_DOT);
            if(getit == SUCCESS){
                addMorseChar('.');
                yay = MORSE_EVENT_NONE;//consuming the event
            }else if(getit == STANDARD_ERROR){
                morseLineClear();
                MorseDecode(MORSE_CHAR_DECODE_RESET);
                yay = MORSE_EVENT_NONE;//consuming the event
            }
            
        }else if(yay == MORSE_EVENT_DASH){ //print dash, decode dash
            getit = MorseDecode(MORSE_CHAR_DASH);
            if(getit == SUCCESS){
                addMorseChar('-');
                yay = MORSE_EVENT_NONE;//consuming the event
            }else if(getit == STANDARD_ERROR){
                morseLineClear();
                MorseDecode(MORSE_CHAR_DECODE_RESET);
                yay = MORSE_EVENT_NONE;//consuming the event
            }
            
        }else if(yay == MORSE_EVENT_INTER_LETTER){ //decode letter, print letter
            getit = MorseDecode(MORSE_CHAR_END_OF_CHAR);
            if(getit != STANDARD_ERROR){
                morseLineClear();
                addLetter(getit);
                yay = MORSE_EVENT_NONE;//consuming the event
                //reset to base root in Tree
                getit = MorseDecode(MORSE_CHAR_DECODE_RESET);
                if(getit == SUCCESS){
                    yay = MORSE_EVENT_NONE;//consuming the event
                }else if(getit == STANDARD_ERROR){
                    morseLineClear();
                    MorseDecode(MORSE_CHAR_DECODE_RESET);
                    yay = MORSE_EVENT_NONE;//consuming the event 
                }
            }else if(getit == STANDARD_ERROR){
                morseLineClear();
                addLetter('#');
                yay = MORSE_EVENT_NONE;
                
                getit = MorseDecode(MORSE_CHAR_DECODE_RESET);
                if(getit == SUCCESS){
                    yay = MORSE_EVENT_NONE;//consuming the event
                }else if(getit == STANDARD_ERROR){
                    morseLineClear();
                    MorseDecode(MORSE_CHAR_DECODE_RESET);
                    yay = MORSE_EVENT_NONE;//consuming the event 
                }
            }
        
        }else if(yay == MORSE_EVENT_INTER_WORD){ //decode letter, print letter, print space
            getit = MorseDecode(MORSE_CHAR_END_OF_CHAR);
            if(getit != STANDARD_ERROR){
                morseLineClear();
                addLetter(getit);
                addLetter(' ');
                yay = MORSE_EVENT_NONE;//consuming the event
                //reset to base root in Tree
                getit = MorseDecode(MORSE_CHAR_DECODE_RESET);
                if(getit == SUCCESS){
                    yay = MORSE_EVENT_NONE;//consuming the event
                }else if(getit == STANDARD_ERROR){
                    morseLineClear();
                    MorseDecode(MORSE_CHAR_DECODE_RESET);
                    yay = MORSE_EVENT_NONE;//consuming the event 
                }
            }else if(getit == STANDARD_ERROR){
                morseLineClear();
                addLetter('#');
                addLetter(' ');
                yay = MORSE_EVENT_NONE;
                
                getit = MorseDecode(MORSE_CHAR_DECODE_RESET);
                if(getit == SUCCESS){
                    yay = MORSE_EVENT_NONE;//consuming the event
                }else if(getit == STANDARD_ERROR){
                    morseLineClear();
                    MorseDecode(MORSE_CHAR_DECODE_RESET);
                    yay = MORSE_EVENT_NONE;//consuming the event 
                }
            }
        
        
        }
        
    }
    
/******************************************************************************
 * Your code goes in between this comment and the preceding one with asterisks.
 *****************************************************************************/

    while (1);
}

void __ISR(_TIMER_2_VECTOR, IPL4AUTO) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    //******** Put your code here *************//
    yay = MorseCheckEvents();
    
}

//A static function that clears the top line of the OLED and updates it
static void morseLineClear(void){
    OledDrawString("                     "); //Clear =)
    x = -6;
    OledUpdate();
}

//A static function that appends a character to the top line and updates OLED
static void addMorseChar(char letter){
    OledDrawChar(x+=6, 0, letter); //drawing chars
    OledUpdate();
}

//A static function that appends a character to the bottom line and updates OLED
static void addLetter(char letter){
    if(strlen(bottom) == 64){
        static int i;
        for(i = 1; i < 64; i++){ //iterate through the loop and move all chars up one index
            bottom[i] = bottom[i + 1];
        }
        sprintf(bottom, "%s%c", bottom, letter); //and keep printing
        OledDrawString(bottom);
        OledUpdate();
    }else if(strlen(bottom) != 64){
        sprintf(bottom, "%s%c", bottom, letter); //printing string every time
        OledDrawString(bottom);
        OledUpdate();
    }
    
}