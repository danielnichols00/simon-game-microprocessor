#include <avr/io.h>
#include <stdint.h>
#include "simon.h"

//Writes individual bytes to SPI - two sided displays requires a left and right byte to be sent every 5ms
void spi_write(void)
{
    static uint8_t current_side = 0;

    if (current_side)
    {
        SPI0.DATA = left_byte;
    }
    else
    {
        SPI0.DATA = right_byte;
    }

    // Toggle the current side
    current_side = !current_side;
}