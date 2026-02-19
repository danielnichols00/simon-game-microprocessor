#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "sequence_generator.h"
#include "initiations.h"
#include "peripherals.h"
#include "types.h"
#include "uart.h"

static void state_machine(void);

int main(void)
{ 
    cli();
    buttons_init();
    display_init();
    adc_init();
    pwm_init();
    timers_init();
    port_init();
    spi_init();
    sei();
    
    disable_display();
    state_machine();
}

/**
 *  ========STATE MACHINE========
*/

    //IMPORTANT VARIABLE DECLARATIONS 

    uint32_t state = 0x11713739; //Initialise state with student number
    static uint32_t latestState = 0x11713739; //Store the start of the playing sequence to be reset to
    static uint32_t restartState = 0x11713739; //Store the state to be set to on restart of the sequence
    static uint8_t sequenceLength = 1; //Store the sequence length
    static uint8_t currentStep = 0; //The current step expressed as a 0, 1, 2 or 3
    static uint8_t currentSequenceStep = 0; //Index of the current step in sequence in relation to pushbutton input
    volatile uint32_t elapsed_time = 0; //Keep track of elapsed time for playback delay
    uint8_t left_byte = 0b11111111; //left byte to be transmitted to spi_write
    uint8_t right_byte = 0b01111111; //right byte to be transmitted to spi_write
    volatile State STATE = PLAYING; //Simon is initiated with play sequence to start the game
    volatile uint8_t pb_current_pb = 0; //Stores the number corresponding to the push button being pressed
    volatile uint8_t pb_released = 0; //Keep track of whether a pushbutton has been released


static void state_machine(void) 
{
    //Pushbutton states
    uint8_t pb_state_prev = 0xFF; //Both state initialised to 0
    uint8_t pb_state_curr = 0xFF; //Both state initialised to 0
    uint8_t pb_falling_edge;
    

    while (1) 
    {
        pb_state_prev = pb_state_curr; //Save state from previous iteration
        pb_state_curr = pb_debounced_state; //Read current state
        pb_falling_edge = (pb_state_prev ^ pb_state_curr) & pb_state_prev; //Variables to hold rising and falling edges

        switch (STATE) 
        {
        case PLAYING:

            if (currentSequenceStep == 0) 
            {
                restartState = state;
            }

            latestState = state;
            currentStep = getStep(&state);

            switch (currentStep)
            {
                case 0:
                update_display(0b10111110, 0b01111111);
                break;

                case 1:
                update_display(0b11101011, 0b01111111);
                break;

                case 2:
                update_display(0b11111111, 0b00111110);
                break;

                case 3:
                update_display(0b11111111, 0b01101011);
                break;
            }

            playBuzzerTone(currentStep);
            playbackDelay();
            currentSequenceStep++;

            if (currentSequenceStep == sequenceLength) 
            {
            currentSequenceStep = 0;
            state = restartState;
            STATE = WAITING;
            } 

            break;
        
        case WAITING:

            if (currentSequenceStep == sequenceLength) 
            {
                currentSequenceStep = 0;
                STATE = SUCCESS;
            }

            if (pb_falling_edge & PIN4_bm || pb_falling_edge & PIN5_bm || pb_falling_edge & PIN6_bm || pb_falling_edge & PIN7_bm) 
            {
                if (pb_falling_edge & PIN4_bm) {
                    pb_current_pb = 0;
                } else if (pb_falling_edge & PIN5_bm) {
                    pb_current_pb = 1;
                } else if (pb_falling_edge & PIN6_bm) {
                    pb_current_pb = 2;
                } else if (pb_falling_edge & PIN7_bm) {
                    pb_current_pb = 3;
                }

                if (pb_current_pb || pb_current_pb == 0) {
                    STATE = BUTTON_PRESSED;
                }
            }
            break;

        case BUTTON_PRESSED:

            //updatePlaybackRate();
            playBuzzerTone(pb_current_pb);

            switch (pb_current_pb)
            {
                case 0:
                update_display(0b10111110, 0b01111111);
                break;

                case 1:
                update_display(0b11101011, 0b01111111);
                break;

                case 2:
                update_display(0b11111111, 0b00111110);
                break;

                case 3:
                update_display(0b11111111, 0b01101011);
                break;
            }
            
            currentStep = getStep(&state);
                
            if (currentStep == pb_current_pb) 
            {
                currentSequenceStep++;

                if (currentSequenceStep == sequenceLength) 
                {
                    currentSequenceStep = 0;
                    STATE = SUCCESS;
                    break;
                }
            }
            else if (currentStep != pb_current_pb) 
            {
                currentSequenceStep = 0;
                STATE = FAIL; 
                break;
            }
            
            playbackDelay();
            STATE = WAITING;
            break;
            
            break;

        case SUCCESS:
            
            playbackDelay();

            sequenceLength++;
            state = restartState;
            STATE = PLAYING;

            break;

        case FAIL:

            playbackDelay();
            displayScore(sequenceLength);
                        
            sequenceLength = 1;

            currentSequenceStep = 0;
            state = latestState;

            STATE = PLAYING;

            break;

        default:
            STATE = IDLE;
            stopTone();
            disable_display();
        }
    }
}