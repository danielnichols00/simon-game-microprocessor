#include <avr/io.h>
#include <avr/interrupt.h>
#include "simon.h"
#include "uart.h"

volatile uint8_t pb_debounced_state = 0xFF;

// Timer ISR - samples pushbuttons
ISR(TCB0_INT_vect)
{
    static uint8_t count0 = 0;
    static uint8_t count1 = 0;

    // Sample & detect changes to pushbutton state
    uint8_t pb_sample = PORTA.IN;
    uint8_t pb_changed = pb_sample ^ pb_debounced_state;

    count1 = (count1 ^ count0) & pb_changed;
    count0 = ~count0 & pb_changed;

    pb_debounced_state ^= (count1 & count0);

    spi_write();

    TCB0.INTFLAGS = TCB_CAPT_bm; //clear interupt
}

//Serial Peripheral Interface ISR
ISR(SPI0_INT_vect) 
{
    PORTA.OUTCLR = PIN1_bm; //Sets DISP LATCH net to low (logic 0)
    PORTA.OUTSET = PIN1_bm; //Sets DISP LATCH net to high (logic 1)
    SPI0.INTFLAGS = SPI_IF_bm; //Disable intereupt
}

//1ms ISR for updating Elapsed Time
ISR(TCB1_INT_vect) {
    elapsed_time++;
    TCB1.INTFLAGS = TCB_CAPT_bm;
}