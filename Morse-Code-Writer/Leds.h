/* ************************************************************************** */
//Sloan Liu
//CE13 Lab6 


#include "BOARD.h"
#include <xc.h>


#ifndef LEDS_H  
#define LEDS_H

//Sets the value of TRISE and LATE to all 0's
#define LEDS_INIT() do{ \
    TRISE = 0x00; \
    LATE = 0x00; \
} while(0)

//Gets the value of the LATE register
#define LEDS_GET() LATE

//sets the LATE register TO X
#define LEDS_SET(x) (LATE = x)

#endif


  