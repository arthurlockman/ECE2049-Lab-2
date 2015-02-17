#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"
#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"
#include "peripherals.h"
#include "notes.h"
#include "HEYYEYAAEYAAAEYAEYAA.h"

typedef enum {S_MENU, S_COUNTDOWN, S_PLAY, S_LOSE} state_t;

void swDelay(int numLoops);
int read_push_button();
void configLED1_3(char inbits);
void BuzzerOnFreq(int freq);
int get_touchpad();
void set_touchpad(char states);
void play_note(Note* note);
void swDelayShort(int numLoops);

int note = 0;

void main(void)
{
	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();
    state_t state = S_MENU;

    GrClearDisplay(&g_sContext);
    GrImageDraw(&g_sContext, &he_man_img, 0, 0);
    GrFlush(&g_sContext);
    while(1)
    {
//    	switch(state)
//    	{
//    	case S_MENU:
//    		break;
//    	case S_COUNTDOWN:
//    		break;
//    	case S_PLAY:
//    		break;
//    	case S_LOSE:
//    		break;
//    	}
    	play_note(&HEYYEYAAEYAAAEYAEYAA[note]);
    	if (note >= 104) note = 0;
    	else note++;
    }
}

int read_push_button()
{
	P1SEL &= ~BIT7;
	P2SEL &= ~BIT2;
	P1DIR &= ~BIT7;
	P2DIR &= ~BIT2;
	P1REN |= BIT7;
	P2REN |= BIT2;
	P1OUT &= BIT7;
	P2OUT &= BIT2;
	if ((P1IN & BIT7) == 0 && (P2IN & BIT2) == 0) return 3;
	else if ((P1IN & BIT7) == 0) return 1;
	else if ((P2IN & BIT2) == 0) return 2;
	else return 0;
}

void configLED1_3(char inbits)
{
    P1SEL &= ~BIT0;
    P8SEL &= ~(BIT1 | BIT2);
    P1DIR |= BIT0;
    P8DIR |= (BIT1 | BIT2);
    P1OUT &= ~BIT0;
    P8OUT &= ~(BIT1 | BIT2);
    if ((inbits & BIT0) != 0) P1OUT |= BIT0;
    if ((inbits & BIT1) != 0) P8OUT |= BIT1;
    if ((inbits & BIT2) != 0) P8OUT |= BIT2;
}

void play_note(Note* note)
{
	set_touchpad(note->LED);
	BuzzerOnFreq(note->pitch);
	//Deal with duration.
	int i;
	for (i = 0; i < note->duration; i++)
	{
		swDelayShort(13);
	}
	BuzzerOff();
	set_touchpad(TOUCHPAD_N);
	swDelayShort(2);
}

void set_touchpad(char states)
{
	P1OUT &= ~(TOUCHPAD_1|TOUCHPAD_2|TOUCHPAD_3|TOUCHPAD_4|TOUCHPAD_5);
	P1OUT |= (states);
}

/**
 * @brief Gets the touchpad button that's activated.
 *
 * @return An int, the button pressed. Corresponds to the sprite column.
 */
int get_touchpad()
{
    //Get touchpad button pressed. Returns column that the button corresponds to.
	switch (CapButtonRead())
	{
	case BUTTON_NONE:
		return -1;
	case BUTTON_X:
		return 0;
	case BUTTON_SQ:
		return 1;
	case BUTTON_OCT:
		return 2;
	case BUTTON_TRI:
		return 3;
	case BUTTON_CIR:
		return 4;
	default:
		return -1;
	}
}

/*
 * Enable a PWM-controlled buzzer on P7.5
 * This function makes use of TimerB0.
 * ACLK = 32768Hz
 */
void BuzzerOnFreq(int freq)
{
	// Initialize PWM output on P7.5, which corresponds to TB0.3
	P7SEL |= BIT5; // Select peripheral output mode for P7.5
	P7DIR |= BIT5;

	TB0CTL  = (TBSSEL__ACLK|ID__1|MC__UP);  // Configure Timer B0 to use ACLK, divide by 1, up mode
	TB0CTL  &= ~TBIE; 						// Explicitly Disable timer interrupts for safety

	// Now configure the timer period, which controls the PWM period
	// Doing this with a hard coded values is NOT the best method
	// I do it here only as an example. You will fix this in Lab 2.
	TB0CCR0   = 32768 / freq; 					// Set the PWM period in ACLK ticks
	TB0CCTL0 &= ~CCIE;					// Disable timer interrupts

	// Configure CC register 3, which is connected to our PWM pin TB0.3
	TB0CCTL3  = OUTMOD_7;					// Set/reset mode for PWM
	TB0CCTL3 &= ~CCIE;						// Disable capture/compare interrupts
	TB0CCR3   = TB0CCR0/2; 					// Configure a 50% duty cycle
}

/**
 * @brief Delay a specified number of loops/
 *
 * @param numLoops The number of loops to delay.
 */
void swDelay(int numLoops)
{
    volatile unsigned long int i, j; // volatile to prevent optimization
    // by compiler

    for (j = 0; j < numLoops; j++)
    {
        i = 50000;                   // SW Delay
        while (i > 0)               // could also have used while (i)
            i--;
    }
}

void swDelayShort(int numLoops)
{
    volatile unsigned long int i, j; // volatile to prevent optimization
    // by compiler

    for (j = 0; j < numLoops; j++)
    {
        i = 500;                   // SW Delay
        while (i > 0)               // could also have used while (i)
            i--;
    }
}
