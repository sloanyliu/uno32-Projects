/*
 * File:   Field.c
 * Author: Sloan Liu & Maxwell Bradley
 *
 * Created on March 9, 2018, 12:10 PM
 */

#include "BOARD.h"
#include "xc.h"
#include "Field.h"
#include "Protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * FieldInit() will fill the passed field array with the data specified in positionData. Also the
 * lives for each boat are filled according to the `BoatLives` enum.
 * @param f The field to initialize.
 * @param p The data to initialize the entire field to, should be a member of enum
 *                     FieldPosition.
 */
void FieldInit(Field *f, FieldPosition p)
{
    int i, j; //loop iterators
    for (i = 0; i < FIELD_ROWS; i++) {
        for (j = 0; j < FIELD_COLS; j++) {
            f ->field[i][j] = p; //setting all field positions to be p
        }
    }
    //setting boat lives
    f ->hugeBoatLives = FIELD_BOAT_LIVES_HUGE;
    f ->largeBoatLives = FIELD_BOAT_LIVES_LARGE;
    f ->mediumBoatLives = FIELD_BOAT_LIVES_MEDIUM;
    f ->smallBoatLives = FIELD_BOAT_LIVES_SMALL;
}

/**
 * Retrieves the value at the specified field position.
 * @param f The Field being referenced
 * @param row The row-component of the location to retrieve
 * @param col The column-component of the location to retrieve
 * @return
 */
FieldPosition FieldAt(const Field *f, uint8_t row, uint8_t col)
{
    return f ->field[row][col]; //returning the spot in the array specified by row and col
}

/**
 * This function provides an interface for setting individual locations within a Field struct. This
 * is useful when FieldAddBoat() doesn't do exactly what you need. For example, if you'd like to use
 * FIELD_POSITION_CURSOR, this is the function to use.
 * 
 * @param f The Field to modify.
 * @param row The row-component of the location to modify
 * @param col The column-component of the location to modify
 * @param p The new value of the field location
 * @return The old value at that field location
 */
FieldPosition FieldSetLocation(Field *f, uint8_t row, uint8_t col, FieldPosition p)
{
    static FieldPosition saved;
    saved = f ->field[row][col]; //saving return value
    f ->field[row][col] = p; //setting the field coordinate to p
    return saved;
}

/**
 * FieldAddBoat() places a single ship on the player's field based on arguments 2-5. Arguments 2, 3
 * represent the x, y coordinates of the pivot point of the ship.  Argument 4 represents the
 * direction of the ship, and argument 5 is the length of the ship being placed. All spaces that
 * the boat would occupy are checked to be clear before the field is modified so that if the boat
 * can fit in the desired position, the field is modified as SUCCESS is returned. Otherwise the
 * field is unmodified and STANDARD_ERROR is returned. There is no hard-coded limit to how many
 * times a boat can be added to a field within this function.
 *
 * So this is valid test code:
 * {
 *   Field myField;
 *   FieldInit(&myField,FIELD_POSITION_EMPTY);
 *   FieldAddBoat(&myField, 0, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_SMALL);
 *   FieldAddBoat(&myField, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_MEDIUM);
 *   FieldAddBoat(&myField, 1, 0, FIELD_BOAT_DIRECTION_EAST, FIELD_BOAT_HUGE);
 *   FieldAddBoat(&myField, 0, 6, FIELD_BOAT_DIRECTION_SOUTH, FIELD_BOAT_SMALL);
 * }
 *
 * should result in a field like:
 *  _ _ _ _ _ _ _ _
 * [ 3 3 3       3 ]
 * [ 4 4 4 4     3 ]
 * [             3 ]
 *  . . . . . . . .
 *
 * @param f The field to grab data from.
 * @param row The row that the boat will start from, valid range is from 0 and to FIELD_ROWS - 1.
 * @param col The column that the boat will start from, valid range is from 0 and to FIELD_COLS - 1.
 * @param dir The direction that the boat will face once places, from the BoatDirection enum.
 * @param boatType The type of boat to place. Relies on the FIELD_POSITION_*_BOAT values from the
 * FieldPosition enum.
 * @return TRUE for success, FALSE for failure
 */
uint8_t FieldAddBoat(Field *f, uint8_t row, uint8_t col, BoatDirection dir, BoatType type)
{
    static uint8_t incCol; //incremented column value
    static uint8_t incRow; //incremented row values
    static int i, inc;
    static FieldPosition boot; //the field position to put down if add boat works
    
    if ((row < 0) || (row > FIELD_ROWS - 1)) { //if row is not within range
        return FALSE;
    }
    if ((col < 0) || (col > FIELD_COLS - 1)) { //if col is not in range
        return FALSE;
    }

    //setting the variables for the switch/if
    if (type == FIELD_BOAT_SMALL) {
        inc = 2;
        boot = FIELD_POSITION_SMALL_BOAT;
    } else if (type == FIELD_BOAT_MEDIUM) {
        inc = 3;
        boot = FIELD_POSITION_MEDIUM_BOAT;
    } else if (type == FIELD_BOAT_LARGE) {
        inc = 4;
        boot = FIELD_POSITION_LARGE_BOAT;
    } else if (type == FIELD_BOAT_HUGE) {
        inc = 5;
        boot = FIELD_POSITION_HUGE_BOAT;
    }

    //differentiate based on direction
    switch (dir) {
    case(FIELD_BOAT_DIRECTION_NORTH):
        if((row - inc) < 0){
            return FALSE;
        }else{
            incRow = row - inc; //setting incremented value 
            if (incRow < 0) { //if not in range
                return FALSE;
            } else if (incRow >= 0) { //if in range
                for (i = row; i > incRow - 1; i--) { //check if all spaces are free
                    if (f ->field[i][col] == FIELD_POSITION_EMPTY) {
                        ; //if a space is free, do nothing
                    } else if (f ->field[i][col] != FIELD_POSITION_EMPTY) {
                        return FALSE;
                        break;
                    }
                }
                //placing down the boat
                for (i = row; i > incRow - 1; i--) {
                    FieldSetLocation(f, i, col, boot);
                }
                return TRUE;
            }
        }
        break;

    case(FIELD_BOAT_DIRECTION_SOUTH):
        if((row + inc) > 5){
            return FALSE;
        }else{
            incRow = row + inc; //setting incremented value 
            if (incRow > FIELD_ROWS - 1) { //if not in range
                return FALSE;
            } else if (incRow <= FIELD_ROWS - 1) { //if in range
                for (i = row; i < incRow + 1; i++) { //check if all spaces are free
                    if (f ->field[i][col] == FIELD_POSITION_EMPTY) {
                        ; //if a space is free, do nothing
                    } else if (f ->field[i][col] != FIELD_POSITION_EMPTY) {
                        return FALSE;
                        break;
                    }
                }
                //placing down the boat
                for (i = row; i < incRow + 1; i++) {
                    FieldSetLocation(f, i, col, boot);
                }
                return TRUE;
            }
        }
        break;

    case(FIELD_BOAT_DIRECTION_WEST):
        if((col - 5) < 0){
            return FALSE;
        }else{
            incCol = col - inc; //setting incremented value 
            if (incRow < 0) { //if not in range
                return FALSE;
            } else if (incCol >= 0) { //if in range
                for (i = col; i > incCol - 1; i--) { //check if all spaces are free
                    if (f ->field[row][i] == FIELD_POSITION_EMPTY) {
                        ; //if a space is free, do nothing
                    } else if (f ->field[row][i] != FIELD_POSITION_EMPTY) {
                        return FALSE;
                        break;
                    }
                }
                //placing down the boat
                for (i = col; i > incCol - 1; i--) {
                    FieldSetLocation(f, row, i, boot);
                }
                return TRUE;
            }
        }
        break;

    case(FIELD_BOAT_DIRECTION_EAST):
        if((col + 5) > 9){
            return FALSE;
        }else{
            incCol = col + inc; //setting incremented value 
            if (incCol > FIELD_COLS - 1) { //if not in range
                return FALSE;
            } else if (incCol <= FIELD_COLS - 1) { //if in range
                for (i = col; i < incCol + 1; i++) { //check if all spaces are free
                    if (f ->field[row][i] == FIELD_POSITION_EMPTY) {
                        ; //if a space is free, do nothing
                    } else if (f ->field[row][i] != FIELD_POSITION_EMPTY) {
                        return FALSE;
                        break;
                    }
                }
                //placing down the boat
                for (i = col; i < incCol + 1; i++) {
                    FieldSetLocation(f, row, i, boot);
                }
                return TRUE;
            }
        }
        break;

    default:
        return FALSE;
        break;
    }

    return FALSE;
}


/**
 * This function registers an attack at the gData coordinates on the provided field:
 * 
 * --'f' is updated with a FIELD_POSITION_HIT or FIELD_POSITION_MISS depending on what was at the
 * coordinates indicated in 'gData'. 
 * 
 * --'gData' is also updated with the proper HitStatus value
 * depending on what happened AND the value of that field position BEFORE it was attacked. 
 * 
 * --Finally
 * this function also reduces the lives for any boat that was hit from this attack.
 * @param f The field to check against and update.
 * @param gData The coordinates that were guessed. The HIT result is stored in gData->hit as an
 *               output.
 * @return The data that was stored at the field position indicated by gData before this attack.
 */
FieldPosition FieldRegisterEnemyAttack(Field *f, GuessData *gData)
{
    static FieldPosition retVal;
    retVal = FieldAt(f, gData ->row, gData ->col); //saving return value

    //setting the boat lives and hit status
    if (f ->field[gData ->row][gData ->col] == FIELD_POSITION_SMALL_BOAT) {
        f ->smallBoatLives -= 1;
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        if(f ->smallBoatLives == 0){
            gData ->hit = HIT_SUNK_SMALL_BOAT;
        }else{
            gData ->hit = HIT_HIT;
        }
    } else if (f ->field[gData ->row][gData ->col] == FIELD_POSITION_MEDIUM_BOAT) {
        f ->mediumBoatLives -= 1;
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        if(f ->mediumBoatLives == 0){
            gData ->hit = HIT_SUNK_MEDIUM_BOAT;
        }else{
            gData ->hit = HIT_HIT;
        }
    } else if (f ->field[gData ->row][gData ->col] == FIELD_POSITION_LARGE_BOAT) {
        f ->largeBoatLives -= 1;
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        if(f ->largeBoatLives == 0){
            gData ->hit = HIT_SUNK_LARGE_BOAT;
        }else{
            gData ->hit = HIT_HIT;
        }
    } else if (f ->field[gData ->row][gData ->col] == FIELD_POSITION_HUGE_BOAT) {
        f ->hugeBoatLives -= 1;
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        if(f ->hugeBoatLives == 0){
            gData ->hit = HIT_SUNK_HUGE_BOAT;
        }else{
            gData ->hit = HIT_HIT;
        }
    } else if(f ->field[gData ->row][gData ->col] == FIELD_POSITION_EMPTY){ //if its a miss, then you miss
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_MISS);
        gData ->hit = HIT_MISS;
    } else if(f ->field[gData ->row][gData ->col] == FIELD_POSITION_HIT){
        gData ->hit = HIT_MISS;
    }else if(f ->field[gData ->row][gData ->col] == FIELD_POSITION_MISS){
        gData ->hit = HIT_MISS;
    }
    
    return retVal;
}

/**
 * This function updates the FieldState representing the opponent's game board with whether the
 * guess indicated within gData was a hit or not. If it was a hit, then the field is updated with a
 * FIELD_POSITION_HIT at that position. If it was a miss, display a FIELD_POSITION_EMPTY instead, as
 * it is now known that there was no boat there. The FieldState struct also contains data on how
 * many lives each ship has. Each hit only reports if it was a hit on any boat or if a specific boat
 * was sunk, this function also clears a boats lives if it detects that the hit was a
 * HIT_SUNK_*_BOAT.
 * @param f The field to grab data from.
 * @param gData The coordinates that were guessed along with their HitStatus.
 * @return The previous value of that coordinate position in the field before the hit/miss was
 * registered.
 */
FieldPosition FieldUpdateKnowledge(Field *f, const GuessData *gData)
{
    static FieldPosition retVal;
    retVal = FieldAt(f, gData ->row, gData ->col); //saving the return value

    //checking what the hit status is
    if (gData ->hit == HIT_MISS) {
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_EMPTY);
    } else if (gData ->hit == HIT_HIT) {
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
    } else if (gData ->hit == HIT_SUNK_SMALL_BOAT) {
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        f ->smallBoatLives = 0;
    } else if (gData ->hit == HIT_SUNK_MEDIUM_BOAT) {
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        f ->mediumBoatLives = 0;
    } else if (gData ->hit == HIT_SUNK_LARGE_BOAT) {
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        f ->largeBoatLives = 0;
    } else if (gData ->hit == HIT_SUNK_HUGE_BOAT) {
        FieldSetLocation(f, gData ->row, gData ->col, FIELD_POSITION_HIT);
        f ->hugeBoatLives = 0;
    }

    return retVal;
}

/**
 * This function returns the alive states of all 4 boats as a 4-bit bitfield (stored as a uint8).
 * The boats are ordered from smallest to largest starting at the least-significant bit. So that:
 * 0b00001010 indicates that the small boat and large boat are sunk, while the medium and huge boat
 * are still alive. See the BoatStatus enum for the bit arrangement.
 * @param f The field to grab data from.
 * @return A 4-bit value with each bit corresponding to whether each ship is alive or not.
 */
uint8_t FieldGetBoatStates(const Field *f)
{
    static uint8_t alive = 0x00;

    if (f ->smallBoatLives > 0) { //if small boat is alive
        alive |= FIELD_BOAT_STATUS_SMALL;
    }

    if (f ->mediumBoatLives > 0) { //if medium boat is alive
        alive |= FIELD_BOAT_STATUS_MEDIUM;
    }

    if (f ->largeBoatLives > 0) { //if large boat is alive
        alive |= FIELD_BOAT_STATUS_LARGE;
    }

    if (f ->hugeBoatLives > 0) { //if huge boat is alive
        alive |= FIELD_BOAT_STATUS_HUGE;
    }

    return alive;
}