/*
 * Portconfig.c
 *
 * Created: 17.09.2013 11:57:05
 *  Author: Robert
 */ 
#include <avr/io.h>

// This function initializes all port pins.
void port_init(void)
{	
	// PA = n.c.
	
	/*
	PB0   = Extern-1							= input
	PB1   = Extern-2							= input
	PB2   = Extern-3							= input
	PB3   = Extern-4 (one wire)					= input
	PB4	  = LED2 (PWM)							= output
	PB5   = LED3 (PWM)							= output
	PB6   = FAN2 (PWM, analog 2)				= output
	PB7   = FAN1 (PWM, analog 1)				= output */
	DDRB = 0b11110000;
	
	// PC0-7 = Extern-(15-22)					= input
	DDRC = 0x00;
	
	/*
	PD0   = Extern-25							= input
	PD1   = Extern-23							= input
	PD2   = tacho FAN1							= input
	PD3   = tacho FAN2							= input	*/
	DDRD = 0x00;
	
	/*
	PE0   = RXD									= input
	PE1   = TXD									= output
	PE4   = tacho FAN4							= input
	PE5   = tacho FAN3							= input	*/
	DDRE = 0b00000010;
		
	// PF0-7 = NTC temperature sensors			= input
	DDRF = 0x00;
	
	/*
	PH0	  = Extern-26							= input
	PH1   = Extern-24							= input
	PH3   = Extern-13 (backlight PWM)			= output
	PH4   = FAN4 (Intel-PWM)					= output
	PH5   = FAN3 (Intel-PWM)					= output
	PH6   = LED1 (PWM)							= output */
	DDRH = 0b01111000;
			
	// PL1   = DFM (tachosignal flow meter)		= input */
	DDRL = 0x00;
}