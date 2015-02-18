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

typedef enum {S_MENU, S_COUNTDOWN, S_PLAY, S_LOSE, S_WIN} state_t;

void swDelay(int numLoops);
int read_push_button();
void configLED1_3(char inbits);
void BuzzerOnFreq(int freq);
int get_touchpad();
void set_touchpad(char states);
void play_note(Note* note);
void swDelayShort(int numLoops);
void runtimerA2(void);
void stoptimerA2(void);
void resetGlobals(void);

int note = 0;
unsigned int sixteenths_passed = 0;
volatile unsigned int count = 0;
volatile unsigned int sixteenths = 0;
int countdown_state = 1;
int note_errors = 0;
int total_errors = 0;
int pad;
int dur;
unsigned int _sixteenths;

#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{
	count++;
	if (count % 13 == 0)
		sixteenths++;
}

void main(void)
{
	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

    _BIS_SR(GIE);

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();
    state_t state = S_MENU;

    GrClearDisplay(&g_sContext);
    while(1)
    {
    	switch(state)
    	{
    	case S_MENU:
    		GrClearDisplay(&g_sContext);
			GrStringDrawCentered(&g_sContext, "MSP430 HERO", AUTO_STRING_LENGTH,
			                                 51, 20, TRANSPARENT_TEXT);
			GrStringDrawCentered(&g_sContext, "PRESS S1 TO START", AUTO_STRING_LENGTH,
			                                 51, 40, TRANSPARENT_TEXT);
			GrStringDrawCentered(&g_sContext, "PRESS S2 TO QUIT", AUTO_STRING_LENGTH,
			                                 51, 50, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
    		if (read_push_button() == 1)
    		{
    			state = S_COUNTDOWN;
    			runtimerA2();
    			countdown_state = 1;
    			resetGlobals();
    		}
    		break;
    	case S_COUNTDOWN:
    		GrClearDisplay(&g_sContext);
    		switch (countdown_state)
    		{
    		case 1:
    			GrStringDrawCentered(&g_sContext, "3...", AUTO_STRING_LENGTH,
    			                                 51, 36, TRANSPARENT_TEXT);
    			configLED1_3(BIT0);
    			BuzzerOnFreq(NOTE_C5);
    			break;
    		case 2:
    			GrStringDrawCentered(&g_sContext, "2...", AUTO_STRING_LENGTH,
    			                                 51, 36, TRANSPARENT_TEXT);
    			configLED1_3(BIT0 | BIT1);
    			BuzzerOnFreq(NOTE_G4);
    			break;
    		case 3:
    			GrStringDrawCentered(&g_sContext, "1...", AUTO_STRING_LENGTH,
    			                                 51, 36, TRANSPARENT_TEXT);
    			configLED1_3(BIT0 | BIT1 | BIT2);
    			BuzzerOnFreq(NOTE_E4);
    			break;
    		case 4:
    			GrStringDrawCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH,
    			                                 51, 36, TRANSPARENT_TEXT);
    			set_touchpad(TOUCHPAD_1|TOUCHPAD_2|TOUCHPAD_3|TOUCHPAD_4|TOUCHPAD_5);
    			configLED1_3(~(BIT0 | BIT1 | BIT2));
    			BuzzerOnFreq(NOTE_C4);
    			break;
    		case 5:
    			state = S_PLAY;
    			stoptimerA2();
    			resetGlobals();
    		    GrClearDisplay(&g_sContext);
    		    GrImageDraw(&g_sContext, &he_man_img, 0, 0);
    		    GrFlush(&g_sContext);
    		    BuzzerOff();
    			runtimerA2();
    			break;
    		}
    		GrFlush(&g_sContext);
    		if (sixteenths % 8 == 0)
    			countdown_state++;
    		break;
    	case S_PLAY:
    		play_note(&HEYYEYAAEYAAAEYAEYAA[note]);
    		_sixteenths = sixteenths;
    		dur = HEYYEYAAEYAAAEYAEYAA[note].duration;
    		pad = get_touchpad();
    		if (pad == HEYYEYAAEYAAAEYAEYAA[note].LED)
    			note_errors++;
    		else if (pad >= 0)
    			BuzzerOnFreq(HEYYEYAAEYAAAEYAEYAA[note].pitch - 30);
    		if (_sixteenths - sixteenths_passed == dur)
    		{
    			note++;
    			sixteenths_passed = _sixteenths;
    			BuzzerOff();
    			if (note_errors == 0) total_errors++;
    			note_errors = 0;
    		}
    		if (read_push_button() == 2)
    		{
    			stoptimerA2();
    			state = S_MENU;
    			GrClearDisplay(&g_sContext);
    			BuzzerOff();
    			note = 0;
    		}
    		if (note >= 104)
    			state = S_WIN;
    		if (total_errors >= 40)
    			state = S_LOSE;
    		break;
    	case S_LOSE:
    		GrClearDisplay(&g_sContext);
    		GrImageDraw(&g_sContext, &skeletor_img, 0, 0);
			GrFlush(&g_sContext);
			int i = 0;
			for (i = 0; i < 4; i++)
			{
				play_note(&youlose_song[i]);
				int j;
				for (j = 0; j < youlose_song[i].duration; j++)
					swDelayShort(10);
			}
			GrClearDisplay(&g_sContext);
			swDelay(1);
			BuzzerOff();
			state = S_MENU;
			total_errors = 0;
    		break;
    	case S_WIN:
    		GrClearDisplay(&g_sContext);
    		GrImageDraw(&g_sContext, &win_img, 0, 0);
			GrFlush(&g_sContext);
			swDelay(1);
			state = S_MENU;
			total_errors = 0;
    		break;
    	}
    }
}

void resetGlobals(void)
{
	note = 0;
	sixteenths_passed = 0;
	sixteenths = 0;
	count = 0;
	note_errors = 0;
	total_errors = 0;
	pad = 0;
	dur = 0;
	_sixteenths = 0;
}

void runtimerA2(void)
{
	TA2CTL = TASSEL_1 + MC_1 + ID_0;
	TA2CCR0 = 327;
	TA2CCTL0 = CCIE;
}

void stoptimerA2(void)
{
	TA2CTL = MC_0;
	TA2CCTL0 &= ~CCIE;
	sixteenths = 0;
	count = 0;
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
	set_touchpad(TOUCHPAD_N);
}

void set_touchpad(char states)
{
	P1OUT &= ~(TOUCHPAD_1|TOUCHPAD_2|TOUCHPAD_3|TOUCHPAD_4|TOUCHPAD_5);
	P1OUT |= (states);
}

int get_touchpad()
{
    //Get touchpad button pressed. Returns column that the button corresponds to.
	switch (CapButtonRead())
	{
	case BUTTON_NONE:
		return -1;
	case BUTTON_X:
		return TOUCHPAD_1;
	case BUTTON_SQ:
		return TOUCHPAD_2;
	case BUTTON_OCT:
		return TOUCHPAD_3;
	case BUTTON_TRI:
		return TOUCHPAD_4;
	case BUTTON_CIR:
		return TOUCHPAD_5;
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
