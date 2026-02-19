#include "initiations.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void buttons_init(void)
{
    // Enable pull-up resistors for PBs
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

void display_init(void)
{
    PORTB.DIRSET = PIN1_bm;  //Configure PB1 as output for DISP EN
    PORTB.OUTSET = PIN1_bm; //Set initial state as off
}

void adc_init(void)
{
    //ADC Configuration

    ADC0.CTRLA = ADC_ENABLE_bm; //enable ADC
    ADC0.CTRLB = ADC_PRESC_DIV2_gc; //set prescaler to 2
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc; // 3.3 MHz requires 4 CLK_PER. VDD as ref
    ADC0.CTRLE = 64; //sample duration set to 64
    ADC0.CTRLF = ADC_LEFTADJ_bm; //set to left adjust
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc; //select AIN2
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc; //select 8-bit resolution, single-ended
}

void pwm_init(void)
{
    //PWM Configuration

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc; //Div 1 - no prescaler required
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm; // Single-slope PWM mode, WO0 enable
    PORTB.OUTCLR = PIN0_bm; // Off
    PORTB.DIRSET = PIN0_bm; // Enable PB0 as an output - BUZZER
    TCA0.SINGLE.PER = 1; // set PWM frequency
    TCA0.SINGLE.CMP0 = 0; // Set duty cycle to 0%
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; //Enable TCA0
}

void timers_init(void)
{
    //Timers Configuration

    // 5ms interrupt for pushbutton sampling
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 16667;
    TCB0.INTCTRL = TCB_CAPT_bm;         
    TCB0.CTRLA = TCB_ENABLE_bm;

    // 1ms interrupt for elapsed time
    TCB1.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB1 in periodic interrupt mode
    TCB1.CCMP = 3333;
    TCB1.INTCTRL = TCB_CAPT_bm;
    TCB1.CTRLA = TCB_ENABLE_bm;
}

void port_init(void)
{
    //Buzzer - PIN0 and USART0 TXD - PIN2
    PORTB.DIRSET = PIN0_bm | PIN2_bm;
}

void spi_init(void) 
{
    //SPI Configuration

    PORTA.DIRSET = PIN1_bm; //enable PORTA Pin 1 (DISP LATCH) as output
    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc; // Set multiplexing for SPI pin
    PORTC.DIR |= (PIN0_bm | PIN2_bm); // Set serial clock (PC0) and master output slave input (PC2) as outputs
    SPI0.CTRLA = SPI_MASTER_bm; // sets master mode
    SPI0.CTRLB = SPI_SSD_bm; // Mode 0, client select disable, unbuffered
    SPI0.CTRLA |= SPI_ENABLE_bm; //enables SPI
    SPI0_INTCTRL = SPI_IE_bm; //enable SPI interrupt

}