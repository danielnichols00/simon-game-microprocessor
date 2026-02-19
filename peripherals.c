#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "simon.h"
#include "uart.h"
#include "peripherals.h"
#include "initiations.h"

//DEFINE BUZZER TONES
#define TONE1_PER 10117
#define TONE2_PER 12015
#define TONE3_PER 7587
#define TONE4_PER 20243

//DEFINE MIN/MAX PLAYBACK DELAY
#define MIN_DELAY 250
#define MAX_DELAY 2000

//DEFINE LEFT SIDE DIGITS
#define LEFT_DIGIT_0 0b10001000
#define LEFT_DIGIT_1 0b11101011
#define LEFT_DIGIT_2 0b11000100
#define LEFT_DIGIT_3 0b11000001
#define LEFT_DIGIT_4 0b10100011
#define LEFT_DIGIT_5 0b10010001
#define LEFT_DIGIT_6 0b10010000
#define LEFT_DIGIT_7 0b11001011
#define LEFT_DIGIT_8 0b10000000
#define LEFT_DIGIT_9 0b10000001

//DEFINE RIGHT SIDE DIGITS
#define RIGHT_DIGIT_0 0b00001000
#define RIGHT_DIGIT_1 0b01101011
#define RIGHT_DIGIT_2 0b01000100
#define RIGHT_DIGIT_3 0b01000001
#define RIGHT_DIGIT_4 0b00100011
#define RIGHT_DIGIT_5 0b00010001
#define RIGHT_DIGIT_6 0b00010000
#define RIGHT_DIGIT_7 0b01001011
#define RIGHT_DIGIT_8 0b00000000
#define RIGHT_DIGIT_9 0b00000001

static const uint16_t periods[4] = 
{
    TONE1_PER,
    TONE2_PER,
    TONE3_PER,
    TONE4_PER
};

static const uint16_t leftNumberPatterns[10] = 
{
    LEFT_DIGIT_0,
    LEFT_DIGIT_1,
    LEFT_DIGIT_2,
    LEFT_DIGIT_3,
    LEFT_DIGIT_4,
    LEFT_DIGIT_5,
    LEFT_DIGIT_6,
    LEFT_DIGIT_7,
    LEFT_DIGIT_8,
    LEFT_DIGIT_9
};

static const uint16_t rightNumberPatterns[10] = 
{
    RIGHT_DIGIT_0,
    RIGHT_DIGIT_1,
    RIGHT_DIGIT_2,
    RIGHT_DIGIT_3,
    RIGHT_DIGIT_4,
    RIGHT_DIGIT_5,
    RIGHT_DIGIT_6,
    RIGHT_DIGIT_7,
    RIGHT_DIGIT_8,
    RIGHT_DIGIT_9
};

extern volatile uint8_t selected_tone; //Current tone
volatile uint16_t playback_delay = 250; //Playback delay in ms - default 250
 
//Takes a number (1 - 4) and activates the buzzer with corresponding tone
void playBuzzerTone(const uint8_t tone) 
{
    uint16_t period = periods[tone];

    //Set timer compare buffers
    TCA0.SINGLE.PERBUF = period;
    TCA0.SINGLE.CMP0BUF = period >> 1;
}

//Stops the buzzers tone
void stopTone(void) 
{
    TCA0.SINGLE.CMP0BUF = 0;
}

//Causes a delay between events according to the potentiometer reading and state
void playbackDelay(void) 
{
    TCB1.CTRLA = ~TCB_ENABLE_bm; //Disable the timer
    elapsed_time = 0;
    uint8_t active = 1; //Engage the while loop
    uint8_t delay_stage = 0; 

    while (active) 
    {
        TCB1.CTRLA = TCB_ENABLE_bm; //Enable the timer

        if (delay_stage == 0 && elapsed_time >= playback_delay >> 1) 
        {
            stopTone();

            if (STATE == PLAYING || STATE == BUTTON_PRESSED) 
            {
                disable_display();
            }
            else if (STATE == SUCCESS) 
            {
                update_display(0b10000000, 0b00000000); //Displays 88
            }
            else if (STATE == FAIL) 
            {
                update_display(0b11110111, 0b01110111); //Displays --
            }
            
            delay_stage = 1;
            elapsed_time = 0;
        }
        else if (delay_stage == 1 && elapsed_time >= playback_delay >> 1) 
        {
            active = 0;
        }
    }
}

//Takes two bytes and update the display via spi_write
void update_display(uint8_t lbyte, uint8_t rbyte) 
{
    left_byte = lbyte;
    right_byte = rbyte;
}

//Disable both sides of the display
void disable_display(void) 
{
    left_byte = 0b11111111; //OFF
    right_byte = 0b01111111; //OFF
}

//Updates the playback delay (ms) based on the potentiometer [Not actually used - Caused test failures unfortunately]
void updatePlaybackRate(void) 
{
    ADC0_COMMAND = ADC_START0_bm;

    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) 
    {
        //wait
    }

    playback_delay = MIN_DELAY + ((ADC0.RESULT * (MAX_DELAY - MIN_DELAY)) / 255);
    ADC0.INTFLAGS = ADC_RESRDY_bm;
}


//Modified playbackDelay to instead display the players score -- Sorry, I know this is bad practice as I have reused code, forgive me :(
void displayScore(uint16_t playerScore) 
{
    uint8_t active = 1; //Engage the while loop
    uint8_t delay_stage = 0;
    elapsed_time = 0;

    uint8_t left_digit = playerScore / 10; //Get the left digit to be displayed
    uint8_t right_digit = playerScore % 10; //Get the right digit to be displayed

    while (active) 
    {
        if (delay_stage == 0 && elapsed_time >= playback_delay >> 1) 
        {
            if (left_digit == 0 && playerScore < 10) 
            {
                update_display(0b11111111, rightNumberPatterns[right_digit]); //Display score with left side disabled
            } 
            else 
            {
                update_display(leftNumberPatterns[left_digit], rightNumberPatterns[right_digit]); //Display both sides for double digit scores
            }

            delay_stage = 1;
            elapsed_time = 0;
        }       
        else if (delay_stage == 1 && elapsed_time >= playback_delay >> 1) 
        {
            active = 0;
        }
    }
}