///  @Company
//    Max Bradley and Sloan Liu Corp
//
//  @File ArtificialAgent.c
//  @Summary
//    Contains code for running agent
//
//  @Description
//    Describe the purpose of this file.
//    Lots

#include "BOARD.h"
#include "xc.h"
#include "Agent.h"
#include "Field.h"
#include "Protocol.h"
#include "FieldOled.h"
#include <string.h>
#include <stdlib.h>
#include "CircularBuffer.h"
#include "Oled.h"


static void ArtificialIdiot(const Field *f); //function prototype

typedef struct {
    uint8_t row;
    uint8_t col;
} FieldPoint;

static FieldPoint hitPoint; //where we want to hit

static AgentState agentStates = AGENT_STATE_GENERATE_NEG_DATA;
static NegotiationData nD;
static NegotiationData *nData = &nD;
static NegotiationData onD;
static NegotiationData *opponentNData = &onD;
static GuessData gD = {0, 0, 0};
static GuessData *gData = &gD;
static GuessData theirgD = {0, 0, 0};
static GuessData *theirGData = &theirgD;

static Field FieldStruct;
static Field FieldStruct2;
static Field *theirField = &FieldStruct;
static Field *myField = &FieldStruct2;

static int i;

/**
 * The Init() function for an Agent sets up everything necessary for an agent before the game
 * starts. This can include things like initialization of the field, placement of the boats,
 * etc. The agent can assume that stdlib's rand() function has been seeded properly in order to
 * use it safely within.
 */
void AgentInit(void)
{
    FieldInit(myField, FIELD_POSITION_EMPTY);
    FieldInit(theirField, FIELD_POSITION_UNKNOWN);
    static uint8_t yGuess, xGuess, dirGuess;
    static uint8_t boatAdded = FALSE;

    do {
        xGuess = rand() % FIELD_COLS;
        yGuess = rand() % FIELD_ROWS;
        dirGuess = rand() % 4;
        boatAdded = FieldAddBoat(myField, yGuess, xGuess, dirGuess, FIELD_BOAT_HUGE);
    } while (boatAdded == FALSE);

    boatAdded = FALSE;

    do {
        xGuess = rand() % FIELD_COLS;
        yGuess = rand() % FIELD_ROWS;
        dirGuess = rand() % 4;
        boatAdded = FieldAddBoat(myField, yGuess, xGuess, dirGuess, FIELD_BOAT_LARGE);
    } while (boatAdded == FALSE);

    boatAdded = FALSE;

    do {
        xGuess = rand() % FIELD_COLS;
        yGuess = rand() % FIELD_ROWS;
        dirGuess = rand() % 4;
        boatAdded = FieldAddBoat(myField, yGuess, xGuess, dirGuess, FIELD_BOAT_MEDIUM);
    } while (boatAdded == FALSE);

    boatAdded = FALSE;

    do {
        xGuess = rand() % FIELD_COLS;
        yGuess = rand() % FIELD_ROWS;
        dirGuess = rand() % 4;
        boatAdded = FieldAddBoat(myField, yGuess, xGuess, dirGuess, FIELD_BOAT_SMALL);
    } while (boatAdded == FALSE);

    FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_NONE);
}

/**
 * The Run() function for an Agent takes in a single character. It then waits until enough
 * data is read that it can decode it as a full sentence via the Protocol interface. This data
 * is processed with any output returned via 'outBuffer', which is guaranteed to be 255
 * characters in length to allow for any valid NMEA0183 messages. The return value should be
 * the number of characters stored into 'outBuffer': so a 0 is both a perfectly valid output and
 * means a successful run.
 * @param in The next character in the incoming message stream.
 * @param outBuffer A string that should be transmit to the other agent. NULL if there is no
 *                  data.
 * @return The length of the string pointed to by outBuffer (excludes \0 character).
 */
int AgentRun(char in, char *outBuffer)
{
    static ProtocolParserStatus protocolDecodeReturnValue;
    static AgentEvent agentEvent;
    //DO NOT RUN PROTOCOL DECODE IF THE in is NULL
    if (in == '\0') {
        agentEvent = AGENT_EVENT_NONE;
    } else if (in != '\0') {
        protocolDecodeReturnValue = ProtocolDecode(in, opponentNData, theirGData);
        switch(protocolDecodeReturnValue){
        case PROTOCOL_PARSING_FAILURE:
            agentEvent = AGENT_EVENT_MESSAGE_PARSING_FAILED;
            break;
            
        case PROTOCOL_WAITING:
            agentEvent = AGENT_EVENT_NONE;
            break;
            
        case PROTOCOL_PARSING_GOOD:
            agentEvent = AGENT_EVENT_NONE;
            break;
            
        case PROTOCOL_PARSED_COO_MESSAGE:
            agentEvent = AGENT_EVENT_RECEIVED_COO_MESSAGE;
            break;
            
        case PROTOCOL_PARSED_CHA_MESSAGE:
            agentEvent = AGENT_EVENT_RECEIVED_CHA_MESSAGE;
            break;
            
        case PROTOCOL_PARSED_DET_MESSAGE:
            agentEvent = AGENT_EVENT_RECEIVED_DET_MESSAGE;
            break;
            
        case PROTOCOL_PARSED_HIT_MESSAGE:
            agentEvent = AGENT_EVENT_RECEIVED_HIT_MESSAGE;
            break;
            
        default:
            break;
        }
    }
    
    switch (agentStates) {
    case AGENT_STATE_GENERATE_NEG_DATA:
        //generate negotiation data, send challenge
        ProtocolGenerateNegotiationData(nData); //encodes it
        //ProtocolEncodeChaMessage(outBuffer, nData); //sends my CHA msg
        agentStates = AGENT_STATE_SEND_CHALLENGE_DATA; //state change
        return ProtocolEncodeChaMessage(outBuffer, nData); //sends my CHA msg

    case AGENT_STATE_SEND_CHALLENGE_DATA:
        if (agentEvent == AGENT_EVENT_RECEIVED_CHA_MESSAGE) {
            //CHA msg fully received
            //now we have opponent's encryptedGuess and hash in opponentNData
            //send my determine data
            //ProtocolEncodeDetMessage(outBuffer, nData); //sent My Det message        
            agentStates = AGENT_STATE_DETERMINE_TURN_ORDER;
            return ProtocolEncodeDetMessage(outBuffer, nData); //sent My Det message       
        } else if (agentEvent == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            agentStates = AGENT_STATE_INVALID;
            OledClear(0);
            OledDrawString(AGENT_ERROR_STRING_PARSING); //error message
            OledUpdate();
        }
        break;

    case AGENT_STATE_DETERMINE_TURN_ORDER:
        //receive opponent's DET msg
        if (agentEvent == AGENT_EVENT_RECEIVED_DET_MESSAGE) {
            //DET msg fully received
            //record opponent data, now have opponent's: guess and encryptionKey in opponentNData
            //validate opponent's data
            if ((ProtocolValidateNegotiationData(opponentNData)) == TRUE) {
                //if data validates, then decide if won turn ordering or not, or if its a tie
                //now figure out which state to switch to if turn order is whatever
                if ((ProtocolGetTurnOrder(nData, opponentNData)) == TURN_ORDER_START) { //I go
                    agentStates = AGENT_STATE_SEND_GUESS;
                    FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_MINE);
                    break;
                } 
                
                if ((ProtocolGetTurnOrder(nData, opponentNData)) == TURN_ORDER_DEFER) { //Opponent goes
                    agentStates = AGENT_STATE_WAIT_FOR_GUESS;
                    FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_THEIRS);
                    break;
                } 
                
                if ((ProtocolGetTurnOrder(nData, opponentNData)) == TURN_ORDER_TIE) { //tie is failure
                    agentStates = AGENT_STATE_INVALID;
                    OledClear(0);
                    OledDrawString(AGENT_ERROR_STRING_ORDERING); //error message
                    OledUpdate();
                    break;
                }
            } else if ((ProtocolValidateNegotiationData(opponentNData)) == FALSE) { //Validation invalid
                agentStates = AGENT_STATE_INVALID;
                OledClear(0);
                OledDrawString(AGENT_ERROR_STRING_NEG_DATA); //error message
                OledUpdate();
            }
        } else if (agentEvent == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            agentStates = AGENT_STATE_INVALID;
            OledClear(0);
            OledDrawString(AGENT_ERROR_STRING_PARSING); //error message
            OledUpdate();
        }
        break;

    case AGENT_STATE_SEND_GUESS:
        //generate coordinates in guess struct

        //start at very top coordinates

        //go thru field coordinates,
        //if hit one, save and continue going
        //if hit only one, check coordinates on all sides -or was it- randomly check one of the sides

        //if it's a hit and the one next to it isn't, then shoot around it on a random case basis
        //if the next is a hit, then shoot to the next of that

        //if not, shoot randomly
        //'Thinking' Time
        for (i = 0; i < (BOARD_GetPBClock() / 8); i++);
//<><><><><><><<><><><<><><><><><><><><><><><><><><><><><><><><><><><><><><>
        //AI start logic
        ArtificialIdiot(theirField);
        gData ->col = hitPoint.col;
        gData ->row = hitPoint.row;
        //ProtocolEncodeCooMessage(outBuffer, gData);
//<><><><><><><<><><><<><><><><><><><><><><><><><><><><><><><><><><><><><><>
        agentStates = AGENT_STATE_WAIT_FOR_HIT;
        return ProtocolEncodeCooMessage(outBuffer, gData);

    case AGENT_STATE_WAIT_FOR_HIT:
        //receive HIT msg
        if (agentEvent == AGENT_EVENT_RECEIVED_HIT_MESSAGE) {
            //decoded opponents HIT message, and received data: my rowGuess, my colGuess, result of my guess
            FieldUpdateKnowledge(theirField, theirGData); //update knowledge of their field based on our guess
            FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_THEIRS); //now its their turn
            //if their boats are still alive then update the field
            if (AgentGetEnemyStatus() != 0) {
                agentStates = AGENT_STATE_WAIT_FOR_GUESS;
                break;
            } else if (AgentGetEnemyStatus() == 0) { //if you killed all of the opponent's boats
                FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_NONE); //update screen
                agentStates = AGENT_STATE_WON; //we won
                break;
            }
        } else if (agentEvent == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            agentStates = AGENT_STATE_INVALID;
            OledClear(0);
            OledDrawString(AGENT_ERROR_STRING_PARSING); //error message
            OledUpdate();
        }
        break;

    case AGENT_STATE_WAIT_FOR_GUESS:
        //Receives opponent's COO msg
        //protocolDecodeReturnValue = ProtocolDecode(in, opponentNData, theirGData); //decoding
        //now we have opponent's guess inside theirGData struct -> row and col
        //COO msg fully received
        if (agentEvent == AGENT_EVENT_RECEIVED_COO_MESSAGE) { //confirmed COO msg
            FieldRegisterEnemyAttack(myField, theirGData); //see if they hit you, updates field and boat lives
            FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_MINE); //update screen
            
            if (AgentGetStatus() != 0) { //if we still alive
                //send back HIT msg to opponent to inform them of their guess result
                agentStates = AGENT_STATE_SEND_GUESS; //back to my guessing
                FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_MINE); //update screen
                return ProtocolEncodeHitMessage(outBuffer, theirGData);
            }else{
                agentStates = AGENT_STATE_LOST; //we lose
                FieldOledDrawScreen(myField, theirField, FIELD_OLED_TURN_NONE); //update screen
                return ProtocolEncodeHitMessage(outBuffer, theirGData);
            }
            
        } else if (agentEvent == AGENT_EVENT_MESSAGE_PARSING_FAILED) {
            agentStates = AGENT_STATE_INVALID;
            OledClear(0);
            OledDrawString(AGENT_ERROR_STRING_PARSING); //error message
            OledUpdate();
        }
        break;

    case AGENT_STATE_INVALID:
        break;

    case AGENT_STATE_LOST:
        break;

    case AGENT_STATE_WON:
        break;

    default:
        break;
    }

    return 0;
}

/**
 * StateCheck() returns a 4-bit number indicating the status of that agent's ships. The smallest
 * ship, the 3-length one, is indicated by the 0th bit, the medium-length ship (4 tiles) is the
 * 1st bit, etc. until the 3rd bit is the biggest (6-tile) ship. This function is used within
 * main() to update the LEDs displaying each agents' ship status. This function is similar to
 * Field::FieldGetBoatStates().
 * @return A bitfield indicating the sunk/unsunk status of each ship under this agent's control.
 *
 * @see Field.h:FieldGetBoatStates()
 * @see Field.h:BoatStatus
 */
uint8_t AgentGetStatus(void)
{
    return FieldGetBoatStates(myField);
}

/**
 * This function returns the same data as `AgentCheckState()`, but for the enemy agent.
 * @return A bitfield indicating the sunk/unsunk status of each ship under the enemy agent's
 *         control.
 *
 * @see Field.h:FieldGetBoatStates()
 * @see Field.h:BoatStatus
 */
uint8_t AgentGetEnemyStatus(void)
{
    return FieldGetBoatStates(theirField);
}

static void ArtificialIdiot(const Field *f)
{
    do{
        hitPoint.col = rand() % FIELD_COLS;
        hitPoint.row = rand() % FIELD_ROWS;
    }while(FieldAt(f, hitPoint.row, hitPoint.col) != FIELD_POSITION_UNKNOWN);
    
    return;
}
