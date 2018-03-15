// **** Include libraries here ****
// Standard libraries

//CMPE13 Support Library
#include "BOARD.h"
#include "Leds.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries

// **** Set macros and preprocessor directives ****
#define RIGHT 1
#define LEFT 0

// **** Declare any datatypes here ****
typedef struct TimerResult{
    uint8_t event;
    uint8_t value;
}TimerResult;

// **** Define global, module-level, or external variables here ****
static TimerResult EventData; //of type struct TimerResult

// **** Declare function prototypes ****

int main(void)
{
    BOARD_Init();

    // Configure Timer 1 using PBCLK as input. This default period will make the LEDs blink at a
    // pretty reasonable rate to start.
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_8, 0xFFFF);

    // Set up the timer interrupt with a priority of 4.
    INTClearFlag(INT_T1);
    INTSetVectorPriority(INT_TIMER_1_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_1_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T1, INT_ENABLED);

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/
    LEDS_INIT(); //TRISE = 0x00 
                 //LATE = 0x00
    LEDS_SET(0x01); //Turn on LED1
    //Don't stop the looping
    int direction = LEFT;
    uint8_t sub = 0;

    while(1)
    {
        //Setting the directions
        if(LEDS_GET() == 0x01){
            direction = LEFT; //If it hit LED1, then start going left
        }else if(LEDS_GET() == 0x80){
            direction = RIGHT; //If it hit LED8, start going right
        }
        
        //If timer event occurred and direction is left
        if(EventData.event == 1){
            if(direction == LEFT){
                sub = (LEDS_GET() << 1); //Moves on to the next LED to the LEFT
                LEDS_SET(sub);
                EventData.event = 0;
            }else if(direction == RIGHT){
                sub = (LEDS_GET() >> 1); //Moves on to the next LED to the RIGHT
                LEDS_SET(sub);
                EventData.event = 0;
            }
        }else if(EventData.event == 0){
            ;
        }
    }
 
    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

    while (1);
}

/**
 * This is the interrupt for the Timer1 peripheral. During each call it increments a counter (the
 * value member of a module-level TimerResult struct). This counter is then checked against the
 * binary values of the four switches on the I/O Shield (where SW1 has a value of 1, SW2 has a value
 * of 2, etc.). If the current counter is greater than this switch value, then the event member of a
 * module-level TimerResult struct is set to true and the value member is cleared.
 */
void __ISR(_TIMER_1_VECTOR, IPL4AUTO) Timer1Handler(void)
{
    //If the timer has hit the limit
    int i = 0;
    EventData.event = 0;
    if((EventData.value) == (SWITCH_STATES())){
        for(i = 0; i < ((SWITCH_STATES()) * 39125); ++i);
        EventData.event = 1; //Set Timer event to be true
        INTClearFlag(INT_T1); // Clear the interrupt flag.
    }else{
        EventData.value = EventData.value + 1; //Then just increment the counter
    }
}