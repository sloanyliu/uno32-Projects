// **** Include libraries here ****
// Standard libraries

//CMPE13 Support Library
#include "BOARD.h"
#include "Field.h"
#include "Protocol.h"
#include "FieldOled.h"
#include "Agent.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>
#include <stdio.h>
#include <string.h>


// User libraries
#include "Buttons.h"
#include "Oled.h"

// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****
    /**
 * NegotiationData stores all of the data required for negotiating the turn order.
 */
//typedef struct {
//    uint32_t guess;
//    uint32_t encryptionKey;
//    uint32_t encryptedGuess;
//    uint32_t hash;
//} NegotiationData;

/**
 * GuessData is used for exchanging coordinate data along with the HitStatus of that coordinate.
 */
//typedef struct {
//    uint32_t row; // Row of the coordinate that was guessed
//    uint32_t col; // Column of the coordinate guessed
//    uint32_t hit; // Status of this coordinate. Uses HitStatus enum constants.
//} GuessData;

//static NegotiationData nD;
//static NegotiationData *nData = &nD;
//static GuessData gD;
//static GuessData *gData = &gD;

// **** Define any module-level, global, or external variables here ****
//static uint32_t counter;
//static uint8_t buttonEvents;

// **** Declare any function prototypes here ****



int main()
{
    BOARD_Init();

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a 10ms timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, BOARD_GetPBClock() / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);

    // Disable buffering on stdout
    //setbuf(stdout, NULL);

    //ButtonsInit();

//    OledInit();
//
//    // Prompt the user to start the game and block until the first character press.
//    OledDrawString("Press BTN4 to start.");
//    OledUpdate();
//    while ((buttonEvents & BUTTON_EVENT_4UP) == 0);


/******************************************************************************
 * Your code goes in between this comment and the following one with asterisks.
 *****************************************************************************/
//    typedef struct {
//    FieldPosition field[FIELD_ROWS][FIELD_COLS];
//    uint8_t smallBoatLives;
//    uint8_t mediumBoatLives;
//    uint8_t largeBoatLives;
//    uint8_t hugeBoatLives;
//} Field;
//    Field mine;
//    Field yours;
//    Field *m = &mine;
//    Field *y = &yours;
//    FieldInit(m, FIELD_POSITION_EMPTY);
//    FieldInit(y, FIELD_POSITION_UNKNOWN);
    //printf("%d%d%d%d\n", m ->hugeBoatLives, m ->largeBoatLives, m ->mediumBoatLives, m ->smallBoatLives);
    //printf("%d%d%d%d\n", y ->hugeBoatLives, y ->largeBoatLives, y ->mediumBoatLives, y ->smallBoatLives);
//    FieldOledDrawScreen(m, y, FIELD_OLED_TURN_MINE);
//    
//    FieldAddBoat(m, 0, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_SMALL);
//    FieldAddBoat(m, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_MEDIUM);
//    FieldAddBoat(m, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_HUGE); //SHOULD FAIL TO PLACE
//    FieldAddBoat(m, 5, 4, FIELD_BOAT_DIRECTION_NORTH, FIELD_BOAT_HUGE);
//    FieldAddBoat(m, 5, 4, FIELD_BOAT_DIRECTION_NORTH, FIELD_BOAT_HUGE); //SHOULD FAIL TO PLACE
//    FieldAddBoat(m, 4, 5, FIELD_BOAT_DIRECTION_NORTH, FIELD_BOAT_SMALL);
//    FieldAddBoat(m, 1, 6, FIELD_BOAT_DIRECTION_SOUTH, FIELD_BOAT_SMALL);
//    FieldAddBoat(m, 4, 6, FIELD_BOAT_DIRECTION_WEST, FIELD_BOAT_HUGE); //SHOULD FAIL TO PLACE
//    FieldAddBoat(m, 5, 3, FIELD_BOAT_DIRECTION_WEST, FIELD_BOAT_MEDIUM);
//    FieldAddBoat(m, 5, 8, FIELD_BOAT_DIRECTION_NORTH, FIELD_BOAT_LARGE);
//    FieldOledDrawScreen(m, y, FIELD_OLED_TURN_MINE);
//    
//    printf("\n");
//    printf("pos at (0,0): %d\n", FieldAt(m, 0, 0)); //should be 1
//    printf("pos at (1,0): %d\n", FieldAt(m, 1, 0)); //should be 2
//    printf("pos at (0,4): %d\n", FieldAt(m, 0, 4)); //should be 4
//    printf("pos at (5,8): %d\n", FieldAt(m, 5, 8)); //should be 3
//    printf("pos at (3,0): %d\n", FieldAt(m, 3, 0)); //Should be 0
//    printf("pos at (0,0): %d\n", FieldAt(y, 0, 0)); //Should be 6
//    printf("\n");
    
//    FieldSetLocation(y, 1, 3, FIELD_POSITION_HIT);
//    FieldSetLocation(y, 1, 6, FIELD_POSITION_HIT);
//    FieldSetLocation(y, 4, 3, FIELD_POSITION_EMPTY);
//    FieldSetLocation(y, 4, 4, FIELD_POSITION_EMPTY);
//    FieldSetLocation(y, 4, 5, FIELD_POSITION_EMPTY);
//    FieldSetLocation(y, 4, 6, FIELD_POSITION_EMPTY);
//    FieldOledDrawScreen(m, y, FIELD_OLED_TURN_MINE);
//    
     //getting enemy's guess, and updating my field based on their guess
    //returns FieldPosition
//    HIT_MISS = 0, // Indicates that this coordinate was a guess that missed.
//    HIT_HIT = 1, // Indicates that a boat was hit at this coordinate.
//    HIT_SUNK_SMALL_BOAT = 2, // Indicates that the SMALL_BOAT was sunk as a result of this hit.
//    HIT_SUNK_MEDIUM_BOAT = 3, // Indicates that the MEDIUM_BOAT was sunk as a result of this hit.
//    HIT_SUNK_LARGE_BOAT = 4, // Indicates that the LARGE_BOAT was sunk as a result of this hit.
//    HIT_SUNK_HUGE_BOAT = 5, // Indicates that the HUGE_BOAT was sunk as a result of this hit.
    
//    static GuessData gdD = {0, 0, 1};
//    static GuessData gdD2 = {0, 1, 1};
//    static GuessData gdD3 = {0, 2, 2};
//    static GuessData gdD4 = {1, 0, 1};
//    static GuessData gdD5 = {1, 1, 1};
//    static GuessData gdD6 = {1, 2, 1};
//    static GuessData gdD7 = {1, 3, 3};
//    static GuessData gdD8 = {2, 0, 0}; 
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD), m ->smallBoatLives);
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD2), m ->smallBoatLives);
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD2), m ->smallBoatLives); //This line was setting the HIT inside of gdD2 to be MISS
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD3), m ->smallBoatLives);
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD4), m ->mediumBoatLives);
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD5), m ->mediumBoatLives);
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD6), m ->mediumBoatLives);
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD7), m ->mediumBoatLives);
//    printf("Fpos: %d, lives: %d\n", FieldRegisterEnemyAttack(m, &gdD8), m ->largeBoatLives);
//    FieldOledDrawScreen(m, y, FIELD_OLED_TURN_MINE);
// 
//    
//    static GuessData gd = {0, 0, 1};
//    static GuessData gd2 = {0, 1, 1};
//    static GuessData gd3 = {0, 2, 2};
//    static GuessData gd4 = {1, 0, 1};
//    static GuessData gd5 = {1, 1, 1};
//    static GuessData gd6 = {1, 2, 1};
//    static GuessData gd7 = {1, 3, 3};
//    static GuessData gd8 = {2, 0, 0};
//    static GuessData gd9 = {0, 9, 1};
//    static GuessData gd10 = {1, 9, 1};
//    static GuessData gd11 = {2, 9, 1};
//    static GuessData gd12 = {3, 9, 1};
//    static GuessData gd13 = {4, 9, 4};
    //getting enemy's response for my guess, update enemy field on my pic32
    //returns FieldPosition
//    FieldUpdateKnowledge(y, &gd); //hit
//    FieldUpdateKnowledge(y, &gd2); //hit
//    FieldUpdateKnowledge(y, &gd3); //Small boat sink
//    FieldUpdateKnowledge(y, &gd4); //hit
//    FieldUpdateKnowledge(y, &gd5); //hit
//    FieldUpdateKnowledge(y, &gd6); //hit
//    FieldUpdateKnowledge(y, &gd7); //Medium boat sunk
//    FieldUpdateKnowledge(y, &gd8); //miss
//    FieldUpdateKnowledge(y, &gd9); //hit
//    FieldUpdateKnowledge(y, &gd10); //hit
//    FieldUpdateKnowledge(y, &gd11); //hit
//    FieldUpdateKnowledge(y, &gd12); //hit
//    FieldUpdateKnowledge(y, &gd13); //Large boat sunk
//    FieldOledDrawScreen(m, y, FIELD_OLED_TURN_MINE);
//            
    //getting your enemy's boat states
    //return uint8_t
//    static uint8_t bStates;
//    bStates = FieldGetBoatStates(y);
//    printf("\nBoat States: %x\n", bStates);
//            
    
//    FIELD_POSITION_EMPTY = 0   
//    FIELD_POSITION_SMALL_BOAT = 1
//    FIELD_POSITION_MEDIUM_BOAT = 2  
//    FIELD_POSITION_LARGE_BOAT = 3  
//    FIELD_POSITION_HUGE_BOAT = 4,   
//    FIELD_POSITION_MISS = 5
//    FIELD_POSITION_UNKNOWN = 6 
//    FIELD_POSITION_HIT = 7     
//    FIELD_POSITION_CURSOR = 8   
           
//--------------------------------------------------------------
    //PROTOCOL DECODE tests
    static char *n11 = "$CHA,37348,117*46\n"; //Challenge message: encryptedGuess, hash
    static char *n12 = "$DET,9578,46222*66\n"; //Determine message: guess, encryptionKey   
    
    //gData = {guess, encryptionKey, encryptedGuess, hash} --> this hash is the bit-wise XOR of guess and key
    static char *n21 = "$CHA,54104,139*45\n";
    static char *n22 = "$DET,32990,21382*5e\n";       
    static char *n31 = "$CHA,62132,70*79\n";
    static char *n32 = "$DET,52343,16067*50\n";         
    static char *n41 = "$CHA,36027,55*7a\n";
    static char *n42 = "$DET,7321,36898*6e\n";
            
    //static char *h1 = "$HIT,3,8,1*43\n";
    static char *h2 = "$HIT,0,2,0*4b\n";
    static char *h3 = "$HIT,2,3,1*49\n";
    static char *h4 = "$HIT,5,6,4*4e\n";
    static char *h5 = "$HIT,0,3,0*4a\n";
    static char *h6 = "$HIT,1,7,1*4e\n";
    static char *h7 = "$HIT,4,8,0*45\n";
    static char *h8 = "$HIT,5,3,3*4c\n";
    static char *h9 = "$HIT,0,5,0*4c\n";
    static char *h10 = "$HIT,5,6,1*4b\n";
    static char *h11 = "$HIT,1,1,1*48\n";
    static char *h12 = "$HIT,1,0,0*48\n";
    static char *h13 = "$HIT,5,2,5*4b\n";
    static char *h14 = "$HIT,2,8,0*43\n";
    static char *h15 = "$HIT,0,6,0*4f\n";
    static char *h16 = "$HIT,5,9,0*45\n";
    static char *h17 = "$HIT,2,8,2*41\n";
            
    static char *c1 = "$COO,0,2*41\n";
    static char *c2 = "$COO,5,5*43\n";
    static char *c3 = "$COO,1,6*44\n";
    static char *c4 = "$COO,0,4*47\n";
    static char *c5 = "$COO,0,5*46\n";
    static char *c6 = "$COO,1,2*40\n";
    static char *c7 = "$COO,3,8*48\n";
    static char *c8 = "$COO,4,0*47\n";
    static char *c9 = "$COO,1,7*45\n";
    static char *c10 = "$COO,0,8*4b\n";
    static char *c11 = "$COO,2,2*43\n";
    static char *c12 = "$COO,4,1*46\n";
    static char *c13 = "$COO,0,0*43\n";
    static char *c14 = "$COO,2,9*48\n";
    
    static char *bs1 = "$REF,1,2*2a\n"; //wrong msgID
    static char *bs2 = "$COO,6,6*g8\n"; //wrong hex CheckSums
    static char *bs3 = "$HIT,2,3,4,5*24\n"; //one too many arguments
    static char *bs4 = "helloWorld!"; //this is wrong
    static char *bs5 = "$COO,0,\02*41\n";
    
    printf("hey");
    static char *testMe, *testMe2, *testMe3, *testMe4;
    static char outBuff[50];
    //@@@@@@@@@@@@@
    testMe = n11; //CHA
    testMe2 = n12; //DET
    testMe3 = h2; //HIT
    testMe4 = c2; //COO
    //@@@@@@@@@@@@@
    static int i, fsmOUT; //length of outBuff
    
    //AgentInit();
    printf("\nStart Test\n");
    for(i = 0; i < strlen(testMe); i++){ //receiving opponent CHA msg
        fsmOUT = AgentRun(testMe[i], outBuff);
        printf("Message Len: %d\nSends Message: %s\n", fsmOUT, outBuff);
    }
    
    for(i = 0; i < strlen(testMe2); i++){ //receiving opponent DET msg
        fsmOUT = AgentRun(testMe2[i], outBuff);
        printf("Message Len: %d\nSends Message: %s\n", fsmOUT, outBuff);
    }
    
    
    for(i = 0; i < strlen(testMe4); i++){ //receiving opponent HIT msg
        fsmOUT = AgentRun(testMe4[i], outBuff);
        printf("Message Len: %d\nSends Message: %s\n", fsmOUT, outBuff);
    }
    
    for(i = 0; i < strlen(testMe3); i++){ //receiving opponent HIT msg
        fsmOUT = AgentRun(testMe3[i], outBuff);
        printf("Message Len: %d\nSends Message: %s\n", fsmOUT, outBuff);
    }

    
    
//    static char *testMe;
//    //@@@@@@@@@@@@@
//    testMe = h1;
//    //@@@@@@@@@@@@@
//    static int i;
//    static ProtocolParserStatus fsmOUT;
//    printf("\nStart Test\n");
//    for(i = 0; i < strlen(testMe); i++){
//        fsmOUT = ProtocolDecode(testMe[i], nData, gData);
//        printf("%d\n", fsmOUT);
//    }
//    printf("gData struct:\n");
//    printf("gData ->row: %u\n", gData ->row);
//    printf("gData ->col: %u\n", gData ->col);
//    printf("gData ->hit: %u\n", gData ->hit);
//    
//    printf("nData struct:\n");
//    printf("nData ->guess: %u\n", nData ->guess);
//    printf("nData ->encryptionKey: %u\n", nData ->encryptionKey);
//    printf("nData ->encryptedGuess: %u\n", nData ->encryptedGuess);
//    printf("nData ->hash: %u\n", nData ->hash);
//    printf("End Test\n\n");
    //--------------------------------------------------------------
    
    
   //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    //PROTOCOL ENCODE tests
//    static char arr[40], arr2[40], arr3[40], arr4[40];
//    //"$HIT,3,8,1*43\n";
//    static GuessData gg = {3, 8, 1};
//    //"$COO,0,2*41\n";
//    static GuessData gg2 = {0, 2, 0};
//    
//    // "$CHA,37348,117*46\n"; //Challenge message: encryptedGuess, hash
//    // "$DET,9578,46222*66\n"; //Determine message: guess, encryptionKey   
//    static NegotiationData nn = {9578, 46222, 37348, 117};
//    
//    //gData = {guess, encryptionKey, encryptedGuess, hash} --> this hash is the bit-wise XOR of guess and key
//    
//    ProtocolEncodeCooMessage(arr, &gg2);
//    printf("CooMsg: %s\n", arr);
//    
//    ProtocolEncodeHitMessage(arr2, &gg);
//    printf("HitMsg: %s\n", arr2);
//    
//    ProtocolEncodeChaMessage(arr3, &nn);
//    printf("ChaMsg: %s\n", arr3);
//    
//    ProtocolEncodeDetMessage(arr4, &nn);
//    printf("DetMsg: %s\n", arr4);
//    
//    PROTOCOL_PARSING_FAILURE = -1                  
//    PROTOCOL_WAITING = 0            
//    PROTOCOL_PARSING_GOOD = 1            
//    PROTOCOL_PARSED_COO_MESSAGE = 2
//    PROTOCOL_PARSED_HIT_MESSAGE = 3
//    PROTOCOL_PARSED_CHA_MESSAGE = 4                                 
//    PROTOCOL_PARSED_DET_MESSAGE = 5
               
/******************************************************************************
 * Your code goes in between this comment and the preceeding one with asterisks
 *****************************************************************************/

    while (1);
}



/**
 * This is the interrupt for the Timer2 peripheral. It just keeps incrementing a counter used to
 * track the time until the first user input.
 */
void __ISR(_TIMER_2_VECTOR, IPL4AUTO) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    // Increment a counter to see the srand() function.
    //counter++;

    // Also check for any button events
    //buttonEvents = ButtonsCheckEvents();
}