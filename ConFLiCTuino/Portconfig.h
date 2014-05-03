/*
 * Portconfig.h
 *
 * Created: 17.09.2013 11:56:49
 *  Author: Robert
 */ 


#ifndef PORTCONFIG_H_
#define PORTCONFIG_H_

// This function initializes all port pins.
void port_init(void);

//#define SWITCH1 ((PORTD_IN & 0b00010000) == 0)
//#define SWITCH2 ((PORTB_IN & 0b00000001) == 0)



#endif /* PORTCONFIG_H_ */