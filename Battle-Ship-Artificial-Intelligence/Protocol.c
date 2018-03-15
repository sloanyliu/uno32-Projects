/*
 * File:   Protocol.c
 * Author: Maxwell Bradley & Sloan Liu
 *
 * Created on March 9, 2018, 12:10 PM
 */

#include "BOARD.h"
#include "xc.h"
#include "Protocol.h"
#include <stdlib.h>
#include <string.h>

typedef enum ParserStates{
    WAITING,
    RECORDING,
    FIRST_CHECKSUM_HALF,
    SECOND_CHECKSUM_HALF,
    NEWLINE
} ParserStates;

typedef enum FailureMessage{
    ERROR = 19
} FailureMessage;

typedef struct DecoderDataStruct{
    uint8_t index;
    char sentence[PROTOCOL_MAX_PAYLOAD_LEN];
    ParserStates protocolState;
    uint8_t checksum;
} DecoderDataStruct;

static uint8_t ChecksumCalculator(char *stringToXor);
static uint8_t HexChecker(char in);
static DecoderDataStruct DecoderData;
ParserStates protocolState = WAITING;

/**
 * Encodes the coordinate data for a guess into the string `message`. This string must be big
 * enough to contain all of the necessary data. The format is specified in PAYLOAD_TEMPLATE_COO,
 * which is then wrapped within the message as defined by MESSAGE_TEMPLATE. The final length of this
 * message is then returned. There is no failure mode for this function as there is no checking
 * for NULL pointers.
 * @param message The character array used for storing the output. Must be long enough to store the
 *                entire string, see PROTOCOL_MAX_MESSAGE_LEN.
 * @param data The data struct that holds the data to be encoded into `message`.
 * @return The length of the string stored into `message`.
 */
int ProtocolEncodeCooMessage(char *message, const GuessData *data)
{
    char formattedWithoutChecksum[PROTOCOL_MAX_PAYLOAD_LEN];
    char buffer[PROTOCOL_MAX_MESSAGE_LEN];
    static uint32_t col;
    col = data ->col;
    static uint32_t row;
    row = data ->row;
  
    sprintf(formattedWithoutChecksum, PAYLOAD_TEMPLATE_COO, row, col); //first pass msg gen
    if((strlen(formattedWithoutChecksum) <= PROTOCOL_MAX_PAYLOAD_LEN) && 
            (strlen(formattedWithoutChecksum) > 0)){
        static uint8_t checkSum;
        checkSum = ChecksumCalculator(formattedWithoutChecksum); //calc checksum
        sprintf(buffer, MESSAGE_TEMPLATE, formattedWithoutChecksum, checkSum); //full msg gen
        strcpy(message, buffer);
        if((strlen(message) <= PROTOCOL_MAX_MESSAGE_LEN) && (strlen(message) > 0)){
            return strlen(message);
        }else{
            return 0;
        }
    }else{
        return 0;
    }

}

/**
 * Follows from ProtocolEncodeCooMessage above.
 */
int ProtocolEncodeHitMessage(char *message, const GuessData *data)
{
    char formattedWithoutChecksum[PROTOCOL_MAX_PAYLOAD_LEN];
    char buffer[PROTOCOL_MAX_MESSAGE_LEN];
    static uint32_t col;
    col = data->col;
    static uint32_t row;
    row = data->row;
    static uint32_t hit;
    hit = data->hit;

    sprintf(formattedWithoutChecksum, PAYLOAD_TEMPLATE_HIT, row, col, hit); //first pass msg gen
    if((strlen(formattedWithoutChecksum) <= PROTOCOL_MAX_PAYLOAD_LEN) && 
            (strlen(formattedWithoutChecksum) > 0)){
        static uint8_t checkSum;
        checkSum = ChecksumCalculator(formattedWithoutChecksum); //calc checksum
        sprintf(buffer, MESSAGE_TEMPLATE, formattedWithoutChecksum, checkSum); //full msg gen
        strcpy(message, buffer);

        if((strlen(message) <= PROTOCOL_MAX_MESSAGE_LEN) && (strlen(message) > 0)){
            return strlen(message);
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}

/**
 * Follows from ProtocolEncodeCooMessage above.
 */
int ProtocolEncodeChaMessage(char *message, const NegotiationData *data)
{
    char formattedWithoutChecksum[PROTOCOL_MAX_PAYLOAD_LEN];
    char buffer[PROTOCOL_MAX_MESSAGE_LEN];
    static uint32_t encryptedGuess;
    encryptedGuess = data->encryptedGuess;
    static uint32_t hash;
    hash = data->hash;

    sprintf(formattedWithoutChecksum, PAYLOAD_TEMPLATE_CHA, encryptedGuess, hash); //first pass msg gen
    if((strlen(formattedWithoutChecksum) <= PROTOCOL_MAX_PAYLOAD_LEN) && 
            (strlen(formattedWithoutChecksum) > 0)){
        static uint8_t checkSum;
        checkSum = ChecksumCalculator(formattedWithoutChecksum); //calc checksum
        sprintf(buffer, MESSAGE_TEMPLATE, formattedWithoutChecksum, checkSum); //full msg gen
        strcpy(message, buffer);
        
        if((strlen(message) <= PROTOCOL_MAX_MESSAGE_LEN) && (strlen(message) > 0)){
            return strlen(message);
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}

/**
 * Follows from ProtocolEncodeCooMessage above.
 */
int ProtocolEncodeDetMessage(char *message, const NegotiationData *data)
{
    char formattedWithoutChecksum[PROTOCOL_MAX_PAYLOAD_LEN];
    char buffer[PROTOCOL_MAX_MESSAGE_LEN];
    static uint32_t encryptionKey;
    encryptionKey = data->encryptionKey;
    static uint32_t guess;
    guess = data->guess;

    sprintf(formattedWithoutChecksum, PAYLOAD_TEMPLATE_DET, guess, encryptionKey); //first pass msg gen
    if((strlen(formattedWithoutChecksum) <= PROTOCOL_MAX_PAYLOAD_LEN) && 
            (strlen(formattedWithoutChecksum) > 0)){
        static uint8_t checkSum;
        checkSum = ChecksumCalculator(formattedWithoutChecksum); //calc checksum
        sprintf(buffer, MESSAGE_TEMPLATE, formattedWithoutChecksum, checkSum); //full msg gen
        strcpy(message, buffer);
        
        if((strlen(message) <= PROTOCOL_MAX_MESSAGE_LEN) && (strlen(message) > 0)){
            return strlen(message);
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}

/**
 * This function decodes a message into either the NegotiationData or GuessData structs depending
 * on what the type of message is. This function receives the message one byte at a time, where the
 * messages are in the format defined by MESSAGE_TEMPLATE, with payloads of the format defined by
 * the PAYLOAD_TEMPLATE_* macros. It returns the type of message that was decoded and also places
 * the decoded data into either the `nData` or `gData` structs depending on what the message held.
 * The onus is on the calling function to make sure the appropriate structs are available (blame the
 * lack of function overloading in C for this ugliness).
 *
 * PROTOCOL_PARSING_FAILURE is returned if there was an error of any kind (though this excludes
 * checking for NULL pointers), while
 * 
 * @param in The next character in the NMEA0183 message to be decoded.
 * @param nData A struct used for storing data if a message is decoded that stores NegotiationData.
 * @param gData A struct used for storing data if a message is decoded that stores GuessData.
 * @return A value from the UnpackageDataEnum enum.
 */
ProtocolParserStatus ProtocolDecode(char in, NegotiationData *nData, GuessData *gData)
{
    uint8_t hexCheckedValue = HexChecker(in);
    char messageId[3] = "";
    switch (DecoderData.protocolState) {
    case WAITING:
        if (in == '$') {
            memset(DecoderData.sentence, '\0', DecoderData.index);
            DecoderData.index = 0;
            DecoderData.protocolState = RECORDING;
            return PROTOCOL_PARSING_GOOD;
        } else {
            return PROTOCOL_WAITING;
        }
        break;
        
    case RECORDING:
        if (in != '*') {
            DecoderData.sentence[DecoderData.index] = in; //not sure if using pointer as an array without declaring size will cause an error?
            DecoderData.index++;
            return PROTOCOL_PARSING_GOOD;
        } else {
            DecoderData.protocolState = FIRST_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        }
        break;
        
    case FIRST_CHECKSUM_HALF:
        if (hexCheckedValue == ERROR) {//if check sum is not a hex value
            DecoderData.protocolState = PROTOCOL_WAITING;
            return PROTOCOL_PARSING_FAILURE;
        } else {
            DecoderData.checksum = (hexCheckedValue << 4); //shifts converted hex value up 4 bits
            DecoderData.protocolState = SECOND_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        }
        break;
        
    case SECOND_CHECKSUM_HALF:
        DecoderData.checksum |= (hexCheckedValue);
        //printf("saved checksum: %u\ngen checksum: %u\nsent: %s\n", DecoderData.checksum, ChecksumCalculator(DecoderData.sentence), DecoderData.sentence);
        if ((hexCheckedValue == ERROR) || (DecoderData.checksum != ChecksumCalculator(DecoderData.sentence))){
            DecoderData.protocolState = PROTOCOL_WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }else{
            DecoderData.sentence[DecoderData.index] = '\0'; //if valid, complete sentence
            DecoderData.protocolState = NEWLINE;
            return PROTOCOL_PARSING_GOOD;
        }
        break;
        
    case NEWLINE:
        if(strlen(DecoderData.sentence) > PROTOCOL_MAX_PAYLOAD_LEN){
            DecoderData.protocolState = WAITING;
            return PROTOCOL_PARSING_FAILURE;
            break;
        }
        if(in == '\n'){ //once end of msg is received, save the top 3 chars of sentence
            sprintf(messageId, "%c", DecoderData.sentence[0]);
            sprintf(messageId, "%s%c", messageId, DecoderData.sentence[1]);
            sprintf(messageId, "%s%c", messageId, DecoderData.sentence[2]);
            
            static uint8_t cha;
            cha = (strcmp(messageId, "CHA") == 0); //CHA msg
            static uint8_t hit;
            hit = (strcmp(messageId, "HIT") == 0); //HIT msg
            static uint8_t det;
            det = (strcmp(messageId, "DET") == 0); //DET msg
            static uint8_t coo;
            coo = (strcmp(messageId, "COO") == 0); //COO msg
            
            static int returnValue;
            
            static char *dec, *temp;
            dec = DecoderData.sentence; //saving the sentence for strtok()
            strtok(dec, ","); //ignore first token
            //set return value to whichever message was passed in
            if (cha) {
                nData ->encryptedGuess = (uint32_t) atof(strtok(NULL, ","));
                temp = strtok(NULL, ",");
                nData ->hash = (uint32_t) atof(strtok(temp , "*")); //removing the '*'
                returnValue = PROTOCOL_PARSED_CHA_MESSAGE;
                DecoderData.protocolState = WAITING;
                return returnValue;
            } else if (hit) {
                gData ->row = (uint8_t) atof(strtok(NULL, ","));
                gData ->col = (uint8_t) atof(strtok(NULL, ","));
                temp = strtok(NULL, ",");
                gData ->hit = (uint8_t) atof(strtok(temp, "*")); //removing the '*'
                returnValue = PROTOCOL_PARSED_HIT_MESSAGE;
                DecoderData.protocolState = WAITING;
                return returnValue;
            } else if (det) {
                nData ->guess = (uint32_t) atof(strtok(NULL, ","));
                temp = strtok(NULL, ",");
                nData ->encryptionKey = (uint32_t) atof(strtok(temp, "*")); //removing the '*'
                returnValue = PROTOCOL_PARSED_DET_MESSAGE;
                DecoderData.protocolState = WAITING;
                return returnValue;
            } else if (coo) {
                gData ->row = (uint8_t) atof(strtok(NULL, ","));
                temp = strtok(NULL, ",");
                gData ->col = (uint8_t) atof(strtok(temp, "*")); //removing the '*'
                returnValue = PROTOCOL_PARSED_COO_MESSAGE;
                DecoderData.protocolState = WAITING;
                return returnValue;
            }else{
                DecoderData.protocolState = WAITING;
                return PROTOCOL_PARSING_FAILURE;
            }
        }else{
            DecoderData.protocolState = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;
        
    default:
        DecoderData.protocolState = WAITING;
        return PROTOCOL_PARSING_FAILURE;
        break;
    }
}


/**
 * This function generates all of the data necessary for the negotiation process used to determine
 * the player that goes first. It relies on the pseudo-random functionality built into the standard
 * library. The output is stored in the passed NegotiationData struct. The negotiation data is
 * generated by creating two random 16-bit numbers, one for the actual guess and another for an
 * encryptionKey used for encrypting the data. The 'encryptedGuess' is generated with an
 * XOR(guess, encryptionKey). The hash is simply an 8-bit value that is the XOR() of all of the
 * bytes making up both the guess and the encryptionKey. There is no checking for NULL pointers
 * within this function.
 * @param data The struct used for both input and output of negotiation data.
 */
void ProtocolGenerateNegotiationData(NegotiationData *data)
{
    static uint8_t p1, p2, p3, p4;
    data ->encryptionKey = rand() & 0x0000FFFF; //make it 16 bit
    data ->guess = rand() & 0x0000FFFF; //make it 16 bit
    
    p1 = (data ->encryptionKey) >> 8; //top 8 bits of key
    p2 = (data ->encryptionKey) & 0x00FF; //bottom 8 bits of key
    p3 = (data ->guess) >> 8; //top 8 bits of guess
    p4 = (data ->guess) & 0x00FF; //bottom 8 bits of guess
    
    data ->hash = p1 ^ p2 ^ p3 ^ p4;
    
    data ->encryptedGuess = (data->encryptionKey) ^ (data->guess);
}

/**
 * Validates that the negotiation data within 'data' is correct according to the algorithm given in
 * GenerateNegotitateData(). Used for verifying another agent's supplied negotiation data. There is
 * no checking for NULL pointers within this function. Returns TRUE if the NegotiationData struct
 * is valid or FALSE on failure.
 * @param data A filled NegotiationData struct that will be validated.
 * @return TRUE if the NegotiationData struct is consistent and FALSE otherwise.
 */
uint8_t ProtocolValidateNegotiationData(const NegotiationData *data)
{
    static uint8_t p1, p2, p3, p4;
    if((data ->encryptedGuess ^ data ->encryptionKey) == data ->guess){
        p1 = (data ->encryptionKey) >> 8; //top 8 bits of key
        p2 = (data ->encryptionKey) & 0x00FF; //bottom 8 bits of key
        p3 = (data ->guess) >> 8; //top 8 bits of guess
        p4 = (data ->guess) & 0x00FF; //bottom 8 bits of guess
        if((p1 ^ p2 ^ p3 ^ p4) == data ->hash){
            return TRUE;
        }else{
            return FALSE;
        }
    }else{
        return FALSE;
    }
}

/**
 * This function returns a TurnOrder enum type representing which agent has won precedence for going
 * first. The value returned relates to the agent whose data is in the 'myData' variable. The turn
 * ordering algorithm relies on the XOR() of the 'encryptionKey' used by both agents. 
 * The least-significant bit of XOR(myData.encryptionKey, oppData.encryptionKey) is checked
 * if it's a 1 
 * the player with the largest 'guess' goes first 
 * otherwise if it's a 0, 
 * the agent with the smallest 'guess' goes first. 
 * The return value of TURN_ORDER_START indicates that 'myData' won,
 * TURN_ORDER_DEFER indicates that 'oppData' won, 
 * otherwise a tie is indicated with TURN_ORDER_TIE.
 * There is no checking for NULL pointers within this function.
 * @param myData The negotiation data representing the current agent.
 * @param oppData The negotiation data representing the opposing agent.
 * @return A value from the TurnOrdering enum representing which agent should go first.
 */
TurnOrder ProtocolGetTurnOrder(const NegotiationData *myData, const NegotiationData *oppData)
{
    static uint16_t xorKeys;
    static uint16_t checkLSB;
    xorKeys = (myData ->encryptionKey) ^ (oppData ->encryptionKey);
    
    checkLSB = xorKeys & 0b0000000000000001;
    if(checkLSB == 0b0){ //smallest guess goes first
        if(myData ->guess > oppData ->guess){
            return TURN_ORDER_DEFER; //oppData goes first
        }else if(myData ->guess < oppData ->guess){
            return TURN_ORDER_START; //myData goes first
        }else if(myData ->guess == oppData ->guess){
            return TURN_ORDER_TIE; //tie
        }      
    }else if(checkLSB == 0b1){ //largest guess goes first
        if(myData ->guess > oppData ->guess){
            return TURN_ORDER_START; //myData goes first
        }else if(myData ->guess < oppData ->guess){
            return TURN_ORDER_DEFER; //oppData goes first
        }else if(myData ->guess == oppData ->guess){
            return TURN_ORDER_TIE; //tie
        }
    }
    
    return TURN_ORDER_TIE;
}

/*Helper #1 
 Calculates CheckSums*/
static uint8_t ChecksumCalculator(char *stringToXor)
{
    static uint8_t totalXor;
    totalXor = stringToXor[0];
    static int i;
    
    for(i = 1; i < strlen(stringToXor); i++){
        totalXor = totalXor ^ stringToXor[i];
    }

    return totalXor;
}

/*Helper #2 
Validates Hex Numbers*/
static uint8_t HexChecker(char in)
{
    //how to check if something is in hex
    switch (in) {
    case ('a'):
        return 0xa;
    case ('b'):
        return 0xb;
    case ('c'):
        return 0xc;
    case ('d'):
        return 0xd;
    case ('e'):
        return 0xe;
    case ('f'):
        return 0xf;
    case ('1'):
    case ('2'):
    case ('3'):
    case ('4'):
    case ('5'):
    case ('6'):
    case ('7'):
    case ('8'):
    case ('9'):
    case ('0'):
        return (in - '0');

    default: 
        return ERROR;
    }

}