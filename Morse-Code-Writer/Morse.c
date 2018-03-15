/*
 * File:   Morse.c
 * Author: Sloan Liu
 *
 * Created on March 2, 2018, 12:54 PM
 */

#include "BOARD.h"
#include "xc.h"
#include "Morse.h"
#include "Tree.h"
#include "Buttons.h"

//States for state machine

typedef enum MorseStates {
    WAITING,
    DOT,
    DASH,
    INTER_LETTER
} MorseStates;


static Node *foreverRoot; //used to get back to top when traversing
static Node *currentRoot; //used to save the current root when traversing
static int freeRunner; //free running counter
static int snapShot; //this is for using with the free runner to tell time elapsed
static MorseStates currentState = WAITING; //used to tell the states for the state machine
static uint8_t buttons; //used to hold ButtonsCheckEvents()

/**
 * This function initializes the Morse code decoder. This is primarily the generation of the
 * Morse tree: a binary tree consisting of all of the ASCII alphanumeric characters arranged
 * according to the DOTs and DASHes that represent each character. Traversal of the tree is done by
 * taking the left-child if it is a dot and the right-child if it is a dash. If the morse tree is
 * successfully generated, SUCCESS is returned, otherwise STANDARD_ERROR is returned. This function
 * also initializes the Buttons library so that MorseCheckEvents() can work properly.
 * @return Either SUCCESS if the decoding tree was successfully created or STANDARD_ERROR if not.
 */
int MorseInit(void)
{
    ButtonsInit();
    //serialization of binary tree
    const char bigDa[63] = "!EISH54V!3UF!!!!2ARL!!!!!WP!!J!1TNDB6!X!!KC!!Y!!MGZ7!Q!!O!8!!90";
    foreverRoot = TreeCreate(6, bigDa); //saving first root
    currentRoot = foreverRoot; //making the base root operational

    //catching TreeCreate() fail
    if (foreverRoot == NULL) {
        return STANDARD_ERROR;
    } else {
        return SUCCESS;
    }
}

/**
 * MorseDecode decodes a Morse string by iteratively being passed MORSE_CHAR_DOT or MORSE_CHAR_DASH.
 * Each call that passes a DOT or DASH returns a SUCCESS if the string could still compose a
 * Morse-encoded character. Passing in the special MORSE_CHAR_END_OF_CHAR constant will terminate
 * decoding and **return the decoded character**. During that call to MorseDecode() the return value
 * will be the character that was decoded or STANDARD_ERROR if it couldn't be decoded. Another
 * special value exists, MORSE_CHAR_DECODE_RESET, which will clear the stored state. When a
 * MORSE_CHAR_DECODE_RESET is done, SUCCESS will be returned. If the input is not a valid MorseChar
 * then the internal state should be reset and STANDARD_ERROR should be returned.
 * 
 * @param in A value from the MorseChar enum which specifies how to traverse the Morse tree.
 * 
 * @return Either SUCCESS on DECODE_RESET or when the next traversal location is still a valid
 *         character, the decoded character on END_OF_CHAR, or STANDARD_ERROR if the Morse tree
 *         hasn't been initialized, the next traversal location doesn't exist/represent a character,
 *         or `in` isn't a valid member of the MorseChar enum.
 */
char MorseDecode(MorseChar in)
{
    if (currentRoot == NULL) {
        return STANDARD_ERROR; //Error when root is NULL
    }

    if (in == MORSE_CHAR_DOT) { //if its a dot, then move to left child
        currentRoot = currentRoot ->leftChild;
        if (currentRoot != NULL) {
            return SUCCESS;
        } else if (currentRoot == NULL) {
            currentRoot = foreverRoot;
            return STANDARD_ERROR;
        }
    } else if (in == MORSE_CHAR_DASH) { //if its a dash, move to right child
        currentRoot = currentRoot ->rightChild;
        if (currentRoot != NULL) {
            return SUCCESS;
        } else if (currentRoot == NULL) {
            currentRoot = foreverRoot;
            return STANDARD_ERROR;
        }
    } else if (in == MORSE_CHAR_END_OF_CHAR) { //do nothing when end of char happens, ready to decode
        if(currentRoot ->data == '!'){
            return STANDARD_ERROR;
        }else{
            ;
        }
    } else if (in == MORSE_CHAR_DECODE_RESET) { //reset = return currentRoot back to base root
        currentRoot = foreverRoot;
        if (currentRoot == NULL) {
            return STANDARD_ERROR;
        } else if (currentRoot != NULL) {
            return SUCCESS;
        }
    }
    return currentRoot ->data; //Decode, will only get to this return if morse char is END_OF_CHAR

}

/**
 * This function calls ButtonsCheckEvents() once per call and returns which, if any,
 * of the Morse code events listed in the enum above have been encountered. It checks for BTN4
 * events in its input and should be called at 100Hz so that the timing works. The
 * length that BTN4 needs to be held down for a dot is >= 0.25s and < 0.50s with a dash being a button
 * down event for >= 0.5s. The button uptime various between dots/dashes (>= .5s), letters
 * (>= 1s), and words (>= 2s).
 *
 * @note This function assumes that the buttons are all unpressed at startup, so that the first
 *       event it will see is a BUTTON_EVENT_*DOWN.
 *
 * So pressing the button for 0.1s, releasing it for 0.1s, pressing it for 0.3s, and then waiting
 * will decode the string '.-' (A). It will trigger the following order of events:
 * 9 MORSE_EVENT_NONEs, 1 MORSE_EVENT_DOT, 39 MORSE_EVENT_NONEs, a MORSE_EVENT_DASH, 69
 * MORSE_EVENT_NONEs, a MORSE_EVENT_END_CHAR, and then MORSE_EVENT_INTER_WORDs.
 * 
 * @return The MorseEvent that occurred.
 */
MorseEvent MorseCheckEvents(void)
{
    freeRunner++; //incrementing free running counter
    buttons = ButtonsCheckEvents(); //saving buttons check events
    
    //************************************************************
    //FSM
    switch (currentState) {
    case(WAITING):
        //Down event means its go time
        if (buttons == BUTTON_EVENT_4DOWN) {
            currentState = DOT;
            snapShot = freeRunner; //take the snapshot
            return MORSE_EVENT_NONE;
        } else {
            currentState = WAITING;
            return MORSE_EVENT_NONE; //return none when nothing happens
        }
        break;

    case(DOT):
        //If time elapsed is less than DOT timeout and button 4 released -> DOT =)
        if ((freeRunner - snapShot) <= MORSE_EVENT_LENGTH_DOWN_DOT) {
            if (buttons == BUTTON_EVENT_4UP) {
                snapShot = freeRunner; //re-snap to reset time detection
                currentState = INTER_LETTER;
                return MORSE_EVENT_DOT;
            } else {
                return MORSE_EVENT_NONE;
            }
        }

        //if time elapsed is greater than dot timeout -> DASH on the ready
        if ((freeRunner - snapShot) > (MORSE_EVENT_LENGTH_DOWN_DOT)) {
            snapShot = 0; //clear the snapshot
            currentState = DASH;
            return MORSE_EVENT_NONE;
        } else {
            return MORSE_EVENT_NONE;
        }

        break;

    case(DASH):
        //if button 4 is released, DASH =)
        if (buttons == BUTTON_EVENT_4UP) {
            snapShot = freeRunner; //re-snap to reset time detection
            currentState = INTER_LETTER;
            return MORSE_EVENT_DASH;
        } else {
            return MORSE_EVENT_NONE; //other wise, forever DASH on the ready 
        }

        break;

    case(INTER_LETTER):
        //when time elapsed is greater than letter time out -> WORD
        if ((freeRunner - snapShot) >= MORSE_EVENT_LENGTH_UP_INTER_LETTER_TIMEOUT) {
            snapShot = 0;
            currentState = WAITING;
            return MORSE_EVENT_INTER_WORD;
            //when time elapsed is more than letter time out and button 4 down -> LETTER
        } else if ((buttons == BUTTON_EVENT_4DOWN) && ((freeRunner - snapShot) >= MORSE_EVENT_LENGTH_UP_INTER_LETTER)) {
            snapShot = freeRunner; //re-snap to reset time detection
            currentState = DOT;
            return MORSE_EVENT_INTER_LETTER;
            //if time elapsed is less then letter time out -> SAME LETTER NOT DONE YET
        } else if ((buttons == BUTTON_EVENT_4DOWN) && (freeRunner - snapShot) < MORSE_EVENT_LENGTH_UP_INTER_LETTER) {
            snapShot = freeRunner; //snapping again for DOT and DASH detection
            currentState = DOT;
            return MORSE_EVENT_NONE;
        } else {
            currentState = INTER_LETTER;
            return MORSE_EVENT_NONE;
        }

        break;
    default:
        //Should not be able to get here
        printf("FATAL ERROR\n");
        while (1) {
            ;
        }
        break;
    }
}

