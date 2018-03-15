// **** Include libraries here ****
// Standard libraries

//CMPE13 Support Library
#include "BOARD.h"
#include "Adc.h"
#include "Ascii.h"
#include "Buttons.h"
#include "Leds.h"
#include "Oled.h"
#include "OledDriver.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

//Functions prototypes

// **** Set any macros or preprocessor directives here ****
// Set a macro for resetting the timer, makes the code a little clearer.
#define TIMER_2HZ_RESET() (TMR1 = 0)
//oh well, hopefully this is ok =)
#define LONG_PRESS 5

// **** Declare any datatypes here ****

typedef enum {
    OFF,
    ON
} ONOFF;

typedef enum {
    TIME,
    TEMP
} INPUT;

typedef enum {
    RESET,
    START,
    COUNT_DOWN,
    PENDING_SELECTOR_CHANGE,
    PENDING_RESET
} STATES;

typedef enum {
    BAKE,
    TOAST,
    BROIL
} MODES;

typedef struct OvenData {
    int cookTimeLeft; //this is in half seconds, to count down in the 2 Hz timer
    uint16_t initCookTime; //This is to save the cook time when we decided to start cooking, this is in seconds
    uint16_t temp; //this is the temperature, it's in degrees fahrenheit 
    MODES currentMode; //this represents the mode the oven is in: bake, toast, or broil
    STATES currentState; //this is the state of the oven in the state machine
    uint16_t buttonPressCounter; //this is my free-running counter used to detect long press
    INPUT inputSelection; //this is my input selector, can be time or temp
} OvenData;

// **** Define any module-level, global, or external variables here ****
static void drawOven(OvenData data);
static void ledsMove(int num, int num2);

static OvenData myOven; //my toaster
static uint8_t buttons; //button event detector
static uint8_t longPressFlag; //to indicate a long press
static uint16_t freeRunner; //storing the free running timer
static ONOFF cookIt; //flag used to decide ON and OFF of an oven
static uint8_t countIt; //saving the initial time for the 2Hz timer to count down
static uint8_t timeDiv8; //the cook time left divided by 8
static uint8_t timer2; //flag sent by the 2Hz timer

//used to keep values when switching input selection
static uint16_t saveHr = 0;
static uint16_t saveMin = 1;
static uint16_t saveTemp = 300;

int main()
{
    BOARD_Init();

    // Configure Timer 1 using PBCLK as input. We configure it using a 1:256 prescalar, so each timer
    // tick is actually at F_PB / 256 Hz, so setting PR1 to F_PB / 256 / 2 yields a 0.5s timer.
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, BOARD_GetPBClock() / 256 / 2);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T1);
    INTSetVectorPriority(INT_TIMER_1_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_1_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T1, INT_ENABLED);

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, BOARD_GetPBClock() / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

    // Configure Timer 3 using PBCLK as input. We configure it using a 1:256 prescalar, so each timer
    // tick is actually at F_PB / 256 Hz, so setting PR3 to F_PB / 256 / 5 yields a .2s timer.
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_256, BOARD_GetPBClock() / 256 / 5);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T3);
    INTSetVectorPriority(INT_TIMER_3_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_3_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T3, INT_ENABLED);

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/
    OledInit();
    ButtonsInit();
    LEDS_INIT();
    AdcInit();
    myOven.currentState = RESET;
    myOven.currentMode = BAKE;
    myOven.inputSelection = TIME;

    while (1) {
        switch (myOven.currentState) {
        case(RESET):
            //making sure lights turn off
            LEDS_SET(0x00);
            myOven.inputSelection = TIME;
            cookIt = OFF;
            drawOven(myOven);
            myOven.currentState = START;
            break;

        case(START):
            myOven.temp = (AdcRead() >> 2) + 300;
            myOven.initCookTime = ((AdcRead() >> 2) + 0b01);
            drawOven(myOven);
            //If button 3 is pressed, that means you want to CHANGE something
            if (buttons == BUTTON_EVENT_3DOWN) {
                freeRunner = myOven.buttonPressCounter;
                myOven.currentState = PENDING_SELECTOR_CHANGE;
            }

            //If you press button 4, then you want to start cooking
            if (buttons == BUTTON_EVENT_4DOWN) {
                TIMER_2HZ_RESET();
                myOven.cookTimeLeft = myOven.initCookTime << 1; //time multiply by 2
                timeDiv8 = myOven.cookTimeLeft >> 3;
                LEDS_SET(0xFF);
                myOven.currentState = COUNT_DOWN;
            }
            break;

        case(COUNT_DOWN):
            cookIt = ON; //flag for oven on
            drawOven(myOven);
            //while loop for counting down
            while (myOven.cookTimeLeft > 0) {
                drawOven(myOven);
                //reacting to the flag
                if (timer2 == 1) {
                    myOven.cookTimeLeft -= 0b1;
                    //moving the led's
                    ledsMove(myOven.cookTimeLeft + timeDiv8, timeDiv8);
                    timer2 = 0; //flag reset
                }
                //time runs out, go to start
                if (myOven.cookTimeLeft == 0) {
                    cookIt = OFF;
                    LEDS_SET(0x00);
                    myOven.currentState = RESET;
                    break;
                }
                //pressing button4 goes to reset pending
                if (buttons == BUTTON_EVENT_4DOWN) {
                    freeRunner = myOven.buttonPressCounter;
                    myOven.currentState = PENDING_RESET;
                    break;
                }
            }
            break;

        case(PENDING_SELECTOR_CHANGE):

            //LONG PRESS -> change input selection
            if (longPressFlag) {
                if (myOven.inputSelection == TIME) {
                    myOven.inputSelection = TEMP;
                } else {
                    myOven.inputSelection = TIME;
                }
                longPressFlag = 0;
                drawOven(myOven);
                myOven.currentState = START; //state change
            }

            //SHORT PRESS
            if (buttons == BUTTON_EVENT_3UP) {
                if (myOven.currentMode == BAKE) {
                    myOven.currentMode = TOAST;
                } else if (myOven.currentMode == TOAST) {
                    myOven.currentMode = BROIL;
                } else if (myOven.currentMode == BROIL) {
                    myOven.currentMode = BAKE;
                }
                myOven.temp = 300;
                myOven.initCookTime = 1;
                longPressFlag = 0;
                drawOven(myOven);
                myOven.currentState = START; //state change
            }
            break;

        case(PENDING_RESET):
            //long press means you want out
            if (longPressFlag) {
                longPressFlag = 0;
                myOven.currentState = RESET;
            }

            //short press does nothing
            if (buttons == BUTTON_EVENT_4UP) {
                longPressFlag = 0;
                myOven.currentState = COUNT_DOWN;
            }
            break;

        default:
            //fatal error catch
            printf("FATAL ERROR!!\n");
            while (1) {
                ;
            }

            break;
        }
    }

    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/
    while (1);
}

//2Hz timer, Free-running, Executes 2 times per second
//This is where we double the initCookTime variable and count down from there

void __ISR(_TIMER_1_VECTOR, ipl4auto) TimerInterrupt2Hz(void)
{
    //Only do stuff when cookIt is ON 
    if (cookIt == ON) {
        timer2 = 1;
    }
    // Clear the interrupt flag.
    IFS0CLR = 1 << 4;
}

void __ISR(_TIMER_3_VECTOR, ipl4auto) TimerInterrupt5Hz(void)
{
    //incrementing the free counter
    myOven.buttonPressCounter += 0b1;
    longPressFlag = 0;
    //if the free counter minus saved time is long press, then long press has occurred
    if ((myOven.buttonPressCounter - freeRunner) == LONG_PRESS) {
        longPressFlag = 1;
        freeRunner = 0;
    }
    // Clear the interrupt flag.
    IFS0CLR = 1 << 12;
}

void __ISR(_TIMER_2_VECTOR, ipl4auto) TimerInterrupt100Hz(void)
{
    //getting the button events delivered 100 times every second
    buttons = ButtonsCheckEvents();
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;
}

//***************************************************
//***************************************************

static void drawOven(OvenData data)
{
    static char a[40];
    static char b[40];
    static char c[40];
    static char d[40];

    if (myOven.currentMode == BAKE) {
        if (cookIt == OFF) {
            OledClear(0);
            sprintf(d, "|\x4\x4\x4\x4\x4|");
            sprintf(a, "|\x2\x2\x2\x2\x2|  Mode: Bake\n"); //Top off 
            if (data.inputSelection == TEMP) {
                sprintf(c, "|-----| >Temp:%4d\xF8%c\n", data.temp, 'F');
                saveTemp = data.temp;
                sprintf(b, "|     |  Time:%2d:%02d\n", saveHr, saveMin);
            } else if (data.inputSelection == TIME) {
                uint16_t hr = data.initCookTime / 60;
                uint16_t min = data.initCookTime % 60;
                sprintf(b, "|     | >Time:%2d:%02d\n", hr, min);
                saveHr = hr;
                saveMin = min;
                sprintf(c, "|-----|  Temp:%4d\xF8%c\n", saveTemp, 'F');
            }
        } else if (cookIt == ON) {
            OledClear(0);
            sprintf(a, "|\x1\x1\x1\x1\x1|  Mode: Bake\n");
            sprintf(b, "|     |  Time:%2d:%02d\n", (data.cookTimeLeft >> 1) / 60, (data.cookTimeLeft >> 1) % 60);
            sprintf(c, "|-----|  Temp:%4d\xF8%c\n", saveTemp, 'F');
            sprintf(d, "|\x3\x3\x3\x3\x3|");
        }
    }

    if (myOven.currentMode == TOAST) {
        if (cookIt == OFF) {
            OledClear(0);
            sprintf(d, "|\x4\x4\x4\x4\x4|");
            sprintf(a, "|\x2\x2\x2\x2\x2|  Mode: Toast\n");
            uint16_t hr = data.initCookTime / 60;
            uint16_t min = data.initCookTime % 60;
            sprintf(b, "|     |  Time:%2d:%02d\n", hr, min);
            saveHr = hr;
            saveMin = min;
            sprintf(c, "|-----|\n");
            myOven.inputSelection = TIME;
        } else if (cookIt == ON) {
            OledClear(0);
            sprintf(a, "|\x2\x2\x2\x2\x2|  Mode: Toast\n");
            sprintf(b, "|     |  Time:%2d:%02d\n", (data.cookTimeLeft >> 1) / 60, (data.cookTimeLeft >> 1) % 60);
            sprintf(c, "|-----|\n");
            sprintf(d, "|\x3\x3\x3\x3\x3|");
            myOven.inputSelection = TIME;
        }
    }

    if (myOven.currentMode == BROIL) {
        if (cookIt == OFF) {
            OledClear(0);
            sprintf(d, "|\x4\x4\x4\x4\x4|");
            sprintf(a, "|\x2\x2\x2\x2\x2|  Mode: Broil\n");
            uint16_t hr = data.initCookTime / 60;
            uint16_t min = data.initCookTime % 60;
            sprintf(b, "|     |  Time:%2d:%02d\n", hr, min);
            saveHr = hr;
            saveMin = min;
            sprintf(c, "|-----|  Temp:%4d\xF8%c\n", 500, 'F');
            myOven.inputSelection = TIME;
        } else if (cookIt == ON) {
            OledClear(0);
            sprintf(a, "|\x1\x1\x1\x1\x1|  Mode: Broil\n");
            sprintf(b, "|     |  Time:%2d:%02d\n", (data.cookTimeLeft >> 1) / 60, (data.cookTimeLeft >> 1) % 60);
            sprintf(c, "|-----|  Temp:%4d\xF8%c\n", 500, 'F');
            sprintf(d, "|\x4\x4\x4\x4\x4|");
            myOven.inputSelection = TIME;
        }
    }

    char into[80]; //the whole oven
    sprintf(into, "%s%s%s%s\n", a, b, c, d);
    OledDrawString(into);
    OledUpdate();
}

static void ledsMove(int num, int num2)
{
    //I know this is dumb but it works 
    if ((num % num2) == 0) {
        if (LEDS_GET() == 0xFF) {
            LEDS_SET(0xFE);
        } else if (LEDS_GET() == 0xFE) {
            LEDS_SET(0xFC);
        } else if (LEDS_GET() == 0xFC) {
            LEDS_SET(0xF8);
        } else if (LEDS_GET() == 0xF8) {
            LEDS_SET(0xF0);
        } else if (LEDS_GET() == 0xF0) {
            LEDS_SET(0xE0);
        } else if (LEDS_GET() == 0xE0) {
            LEDS_SET(0xC0);
        } else if (LEDS_GET() == 0xC0) {
            LEDS_SET(0x80);
        } else if (LEDS_GET() == 0x80) {
            LEDS_SET(0x00);
        }
    }

}