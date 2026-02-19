#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "sequence_generator.h"
#include "simon.h"

/**
 *  Shifts the bits in state by 1 position to the right.
 *  If the bit shifted out of state in step 1 was set (1), take the XOR
 *  of "state" with 0xE2023CAB and store the result in "state".
 * 
 *  Next the least signficant two bits of state are inspected and will return a 1 - 4
*/
uint8_t getStep(uint32_t *pointer) 
{
    uint8_t shiftedBit; //Store a 1 or 0 depending on if a bit is shifted out
    uint8_t leastSignificantBits = 0b11;

    if (state & 0x0001) 
    {
        shiftedBit = 1;
    }
    else 
    {
        shiftedBit = 0;
    }

    state >>= 1;

    if (shiftedBit) 
    {
        state = (state ^ 0xE2023CAB);
    }

    leastSignificantBits = (leastSignificantBits & *pointer);

    if (leastSignificantBits == 0b00) 
    {
        return 0;
    } 
    else if (leastSignificantBits == 0b01) 
    {
        return 1;
    } 
    else if (leastSignificantBits == 0b10) 
    {
        return 2;
    } 
    else if (leastSignificantBits == 0b11) 
    {
        return 3;
    }
    else 
    {
        return 4;
    }
}
