// **** Include libraries here ****
// Standard libraries

//CMPE13 Support Library
#include "BOARD.h"
#include "Buttons.h"
#include "Ascii.h"
#include "Leds.h"
#include "Oled.h"
#include "OledDriver.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries


// **** Set macros and preprocessor directives ****
#define RIGHT 1
#define LEFT 0

typedef struct AdcResult{
    uint8_t event;
    uint16_t value;
}AdcResult;

typedef struct TimerResult{
    uint8_t event;
    int value;
}TimerResult;

// **** Define global, module-level, or external variables here ****
static TimerResult EventData;
static AdcResult adc;
uint8_t buttonEvents;
uint8_t prev;
int fg;

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



    // Enable interrupts for the ADC
    ConfigIntADC10(ADC_INT_PRI_2 | ADC_INT_SUB_PRI_0 | ADC_INT_ON);

    // Set B2 to an input so AN0 can be used by the ADC.
    TRISBCLR = 1 << 2;

    // Configure and start the ADC
    // Read AN0 as sample a. We don't use alternate sampling, so setting sampleb is pointless.
    SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN2);
    OpenADC10(
            ADC_MODULE_ON | ADC_IDLE_CONTINUE | ADC_FORMAT_INTG16 | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_ON,
            ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF | ADC_SAMPLES_PER_INT_8 |
            ADC_BUF_16 | ADC_ALT_INPUT_OFF,
            ADC_SAMPLE_TIME_29 | ADC_CONV_CLK_PB | ADC_CONV_CLK_51Tcy2,
            ENABLE_AN2_ANA,
            SKIP_SCAN_ALL);
    EnableADC10();

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/
    //Part 1 stuff
    LEDS_INIT(); //TRISE = 0x00 
                 //LATE = 0x00
    ButtonsInit();
    LEDS_SET(0x01); //Turn on LED1
    //Don't stop the looping
    int direction = LEFT;
    uint8_t sub = 0;
    //--------
    
    //Part 2 stuff
    OledInit();
    OledDrawString("Potentiometer value:\n");
    OledUpdate();
    //--------

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
        
        //////////////////////////////////////////////
        //Oled display
        if(adc.event == 1){
            char str[1];
            char str2[1];
            sprintf(str, "\n%4d", adc.value);
            OledDrawString(str);
            OledUpdate();
            int joe = (adc.value * 100) /1022;
            sprintf(str2, "\n\n%3d%%", joe);
            OledDrawString(str2);
            OledUpdate();
        //Otherwise dont do anythinggg
        }else if(adc.event == 0){
            ;
        }
        
        if(fg){
            //Using XOR for toggling, so I don't have to worry about turning 
            //operations for button 1
            if((SWITCH_STATES() & 0x01) && (buttonEvents & BUTTON_EVENT_1UP)){
                LEDS_SET(LEDS_GET() ^ 0x03);
            }else if((!(SWITCH_STATES() & 0x01)) && (buttonEvents & BUTTON_EVENT_1DOWN)){
                LEDS_SET(LEDS_GET() ^ 0x03);
            }
             
            //operations for button 2
            if((SWITCH_STATES() & 0x02) && (buttonEvents & BUTTON_EVENT_2UP)){
                LEDS_SET(LEDS_GET() ^ 0x0C);
            }else if((!(SWITCH_STATES() & 0x02)) && (buttonEvents & BUTTON_EVENT_2DOWN)){
                LEDS_SET(LEDS_GET() ^ 0x0C); 
            }

            //operations for button 3
            if((SWITCH_STATES() & 0x04) && (buttonEvents & BUTTON_EVENT_3UP)){
                LEDS_SET(LEDS_GET() ^ 0x30);
            }else if((!(SWITCH_STATES() & 0x04)) && (buttonEvents & BUTTON_EVENT_3DOWN)){
                LEDS_SET(LEDS_GET() ^ 0x30);
            }

            //operations for button 4
            if((SWITCH_STATES() & 0x08) && (buttonEvents & BUTTON_EVENT_4UP)){
                LEDS_SET(LEDS_GET() ^ 0xC0);
            }else if((!(SWITCH_STATES() & 0x08)) && (buttonEvents & BUTTON_EVENT_4DOWN)){
                LEDS_SET(LEDS_GET() ^ 0xC0);
            }
            
            //resetting the flag
            fg = 0;
        }
    }


    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

    while (1);
}

/**
 * This is the interrupt for the Timer1 peripheral. It checks for button events and stores them in a
 * module-level variable. Additionally during each call it increments a counter (the value member of
 * a module-level TimerResult struct). This counter is then checked against the top four bits of the
 * ADC result, and if it's greater, then the event member of a module-level TimerResult struct is
 * set to true and the value member is cleared.
 */
void __ISR(_TIMER_1_VECTOR, IPL4AUTO) Timer1Handler(void)
{
    buttonEvents = ButtonsCheckEvents();
    //Comparing the previous event to the last event that occurred 
    if(prev != buttonEvents){
        fg = 1; //something happened, so flag is up
        prev = buttonEvents; //updating the previous event for comparing again
    }
    
    INTClearFlag(INT_T1);

}

/**
 * This is the ISR for the ADC1 peripheral. It has been enabled to run continuously. Reads all 8
 * samples from the ADC, averages them, and stores them in a module-level variable for use in the
 * main event loop.
 */
void __ISR(_ADC_VECTOR, IPL2AUTO) AdcHandler(void)
{
    
    //getting that avg
    uint16_t tot = 0;
    tot = (ADC1BUF0 + ADC1BUF1 + ADC1BUF2 + ADC1BUF3 + ADC1BUF4 + ADC1BUF5 + ADC1BUF6 + ADC1BUF7);
    uint16_t avg = tot / 8;
    
    //Part 1 Interrupt
//    int i = 0;
//    EventData.event = 0;
//    if((EventData.value) == (SWITCH_STATES())){
//        for(i = 0; i < ((adc.value) * 391); ++i);
//        EventData.event = 1; //Set Timer event to be true
//        INTClearFlag(INT_AD1); // Clear the interrupt flag.
//    }else{
//        EventData.value = EventData.value + 1; //Then just increment the counter
//    }
    
    //securing a base 0
    if((ADC1BUF0 == 0) || (ADC1BUF1 == 0) ||  (ADC1BUF2 == 0) || (ADC1BUF3 == 0) || 
       (ADC1BUF4 == 0) || (ADC1BUF5 == 0) || (ADC1BUF6 == 0) || (ADC1BUF7 == 0)){
        adc.value = 0;
        adc.event = 1;
        
        if((EventData.value) == (adc.value)){
            int i;
            for(i = 0; i < ((adc.value) * 391); ++i);
            EventData.event = 1; //Set Timer event to be true
        }else{
            EventData.event = 0;
            EventData.value = EventData.value + 1; //Then just increment the counter
        }
        
        INTClearFlag(INT_AD1);
    //securing a base for 1023
    }else if((ADC1BUF0 == 1023) || (ADC1BUF1 == 1023) ||  (ADC1BUF2 == 1023) || (ADC1BUF3 == 1023) 
         || (ADC1BUF4 == 1023) || (ADC1BUF5 == 1023) || (ADC1BUF6 == 1023) || (ADC1BUF7 == 1023)){
        adc.value = 1023;
        adc.event = 1;
        
        if((EventData.value) == (adc.value)){
            int i;
            for(i = 0; i < ((adc.value) * 391); ++i);
            EventData.event = 1; //Set Timer event to be true
        }else{
            EventData.event = 0;
            EventData.value = EventData.value + 1; //Then just increment the counter
        }
        
        INTClearFlag(INT_AD1);
    //the check conditions are for avoiding flickering on the OLED when values spastically change up and down by 1
    }else if((avg == adc.value) || ((avg - 1) == adc.value) || ((avg + 1) == adc.value)){
        adc.event = 0;
        EventData.event = 0;
    }else if((avg != adc.value) || ((avg - 1) != adc.value) || ((avg + 1) != adc.value)){
        adc.value = avg;
        adc.event = 1;
        
        if((EventData.value) == (adc.value)){
            int i;
            for(i = 0; i < ((adc.value) * 391); ++i);
            EventData.event = 1; //Set Timer event to be true
        }else{
            EventData.event = 0;
            EventData.value = EventData.value + 1; //Then just increment the counter
        }
        
        INTClearFlag(INT_AD1);
    }
}