/*********************************************************************/
/**
 * @file Timer.c
 * @brief Timer depended functions
 * @author Robert Steigemann
 *
 * project: ConFLiCT
 *********************************************************************/


#include "Timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ConFLiCTuino.h"


#define R_CH1 OCR1C					// 0-319
#define R_CH2 OCR1B					// 0-319
#define R_CH3 OCR4C					// 0-319
#define R_CH4 OCR4B					// 0-319

#define R_LED_GREEN OCR1A			// 0-319
#define R_LED_BLUE OCR2A			// 0-255
#define R_LED_RED OCR2B				// 0-255

#define R_BACKLIGHT OCR4A			// 0-319

static volatile uint16_t period_flowMeter = 0;
static volatile uint16_t oldCount_flowMeter = 0;
static volatile uint8_t status_flowMeter = 2;
static volatile uint8_t enable_flowMeter = 0;

static volatile uint16_t period_CH[4] = {0, 0, 0, 0};
static volatile uint16_t oldCount_CH[4] = {0, 0, 0, 0};
static volatile uint8_t status_CH[4] = {2, 2, 2, 2};
static volatile uint8_t enable_CH[4] = {2, 2, 2, 2};	// 0 means enabled
	
static volatile uint8_t timer524msTick = 0;
static volatile uint8_t timer100msTick = 0;
static volatile uint8_t tickscaler = 0;

static volatile	uint16_t impulsePerLiter = 169;


// This function initializes the timers TC1 & TC4 for 25.000 kHz PWM.
// TC2 to an other PWM frequency (>20 kHz), TC5 for period measuring and TC 3 for tick generation
void timer_init()
{
	// Three timers for PWM generation.
	TCCR1A = 0b10101000;			// clear up, set down, phase and frequency correct PWM
	ICR1 = (320 -1);				// 16MHz / (2*320) = 25kHz
									// maximum is 319 so 0 - 319 are allowed values	
	TCCR1B = 0b00010001;			// WGM: 8, 16 MHz
	
	
	TCCR4A = 0b10101000;			// clear up, set down, phase and frequency correct PWM
	ICR4 = (320 -1);				// 16MHz / (2*320) = 25kHz
									// maximum is 319 so 0 - 319 are allowed values	
	TCCR4B = 0b00010001;			// WGM: 8, 16 MHz
	
	
	TCCR2A = 0b10100001;			// clear up, set down, phase correct PWM
									// 16MHz / (2*256) = 31.25kHz
									// maximum is 255 so 0 - 255 are allowed values
	TCCR2B = 0b00000001;			// WGM: 8, 16 MHz
	
	
		
	// TC5 for period measuring
	TCCR5A = 0b00000000;			// no output compare WGM: normal
	TCCR5B = 0b10000100;			// input noise cancel, falling edge, prescaler = 256
	TIMSK5 = 0b00100001;			// input capture and overflow interrupt enabled
	
	EICRA = 0b10100000;				// INT2 and INT3 to falling edge
	EICRB = 0b00001010;				// INT4 and INT5 to falling edge
	EIMSK = 0b00111100;				// INT2-5 enable
	
	
	// TC3 for 100 ms trigger
	TCCR3A = 0b00000000;			// no output compare WGM: 12 CTC at ICR3
	OCR3A = 6249;					// 16MHz / (256*6250) = 10Hz
	TCCR3B = 0b00001100;			// prescaler = 256
	TIMSK3 = 0b00000010;			// overflow interrupt enabled		
}


ISR(TIMER5_CAPT_vect)			// Flow meter
{
	uint16_t aValue;
	
	aValue = ICR5;		// Get the counter Value
	
	if (status_flowMeter < 2)
	{
		period_flowMeter = aValue - oldCount_flowMeter;
	}
	
	oldCount_flowMeter = aValue;
	
	if (status_flowMeter > 0)
	{
		status_flowMeter --;
	}	
}

ISR(INT2_vect)			// CH1
{
	uint16_t aValue;
	
	aValue = TCNT5;		// Get the counter Value
	
	if (status_CH[0] < 2)
	{
		period_CH[0] = aValue - oldCount_CH[0];
	}
	
	oldCount_CH[0] = aValue;
	
	if (status_CH[0] > 0)
	{
		status_CH[0] --;
	}	
}

ISR(INT3_vect)			// CH2
{
	uint16_t aValue;
	
	aValue = TCNT5;		// Get the counter Value
	
	if (status_CH[1] < 2)
	{
		period_CH[1] = aValue - oldCount_CH[1];
	}
	
	oldCount_CH[1] = aValue;
	
	if (status_CH[1] > 0)
	{
		status_CH[1] --;
	}
}

ISR(INT5_vect)			// CH3
{
	uint16_t aValue;
	
	aValue = TCNT5;		// Get the counter Value
	
	if (status_CH[2] < 2)
	{
		period_CH[2] = aValue - oldCount_CH[2];
	}
	
	oldCount_CH[2] = aValue;
	
	if (status_CH[2] > 0)
	{
		status_CH[2] --;
	}
}

ISR(INT4_vect)			// CH4
{
	uint16_t aValue;
	
	aValue = TCNT5;		// Get the counter Value
	
	if (status_CH[3] < 2)
	{
		period_CH[3] = aValue - oldCount_CH[3];
	}
	
	oldCount_CH[3] = aValue;
	
	if (status_CH[3] > 0)
	{
		status_CH[3] --;
	}
}

ISR(TIMER5_OVF_vect)			// Each 1048.576 ms
{
	enable_flowMeter = status_flowMeter;	
	status_flowMeter = 2;
	
	for (uint8_t i = 0;i<4;i++)
	{
		enable_CH[i] = status_CH[i];
		status_CH[i] = 2;
	}
}

ISR(TIMER3_COMPA_vect)			// Each 100.000 ms
{
	timer100msTick = 1;		// set the tick
	
	if (tickscaler == 4)
	{
		timer524msTick = 1;		// set the tick
		tickscaler = 0;
	}
	else
	{
		tickscaler ++;
	}
}

void timer_setPwm(uint8_t channel, uint8_t pwmValue)
{
	// check if pwmValue is a valid percent number
	if (pwmValue < 101)
	{
						
		switch (channel)					// select the channel
		{
			case 0:
				R_CH1 = (319*pwmValue)/100;	// change the value
				break;
			case 1:
				R_CH2 = (319*pwmValue)/100;
				break;
			case 2:
				R_CH3 = 319-(319*pwmValue)/100;
				break;
			case 3:
				R_CH4 = 319-(319*pwmValue)/100;
				break;			
			case LED_RED:
				R_LED_RED = (uint8_t)((255*pwmValue)/100);		
				break;
			case LED_GREEN:
				R_LED_GREEN = (319*pwmValue)/100;
				break;
			case LED_BLUE:
				R_LED_BLUE = (uint8_t)((255*pwmValue)/100);
				break;
			case BACKLIGHT:
				R_BACKLIGHT = (319*pwmValue)/100;
				break;
		}
	}		
}


// Get rotation per minute of a fan. A fan makes two impulses per rpm!
uint16_t timer_getRpm(uint8_t channel)
{
	if (enable_CH[channel] == 0)	// Channel is enabled.
	{
		uint32_t dummy = (uint32_t)period_CH[channel] * 2;
		return ((uint16_t)(3750000UL / dummy));			
	} 
	else							// Channel is disabled.
	{
		return 0;
	}
}

// Get liters per hour of the flow meter.
uint16_t timer_getLH(void)
{
	if (enable_flowMeter == 0)		// Flow meter is enabled
	{
		uint32_t dummy = (uint32_t)period_flowMeter * impulsePerLiter;
		return (uint16_t)(225000000UL / dummy);		
	} 
	else
	{
		return 0;
	}
}

void timer_setImpulsePerLiter(uint16_t aValue)
{
	impulsePerLiter = aValue;	
}


uint8_t timer_524ms(void)
{
	if (timer524msTick)
	{
		timer524msTick = 0;
		return 1;
	} 
	else
	{
		return 0;
	}	
}


uint8_t timer_100ms(void)
{
	if (timer100msTick)
	{
		timer100msTick = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}