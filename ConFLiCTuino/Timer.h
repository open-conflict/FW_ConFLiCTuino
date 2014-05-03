/*********************************************************************/
/**
 * @file Timer.h
 * @brief Timer depended functions
 * @author Robert Steigemann
 *
 * project: ConFLiCT
 *
 * @date 06.10.2013 - first implementation
 * @date 23.02.2014 - tests pass, working fine
 *********************************************************************/

#include <avr/io.h>

#ifndef TIMER_H_
#define TIMER_H_


#define LED_RED 4
#define LED_GREEN 5
#define LED_BLUE 6
#define BACKLIGHT 7

/**
 * @brief Initializes the timers TCE0 & TCD0 for 25 kHz PWM, TCC0 for pulse measure and TCC1 for 100ms timestamps
*/
void timer_init(void);

/**
 * @brief Set the power of a PWM output
 * @param channel the channel which is set (0-7)\n
 *				  0 - 3 for the fan channels\n
 *				  other channels see defines
 * @param pwmValue the power in percent (0-100)
 *
 * Change the output duty cycle from 0 to 100 % of the specified channel.
*/
void timer_setPwm(uint8_t channel, uint8_t pwmValue);

/**
* @brief Get rotation per minute of a fan
* @param channel the channel to get the rpm from (0-3)
* @return uint16_t the rpm of the fan connected to the channel
*/
uint16_t timer_getRpm(uint8_t channel);

/**
* @brief Set impulse per liter for liter per hour calculation
* @param aValue The amount of impulses returned from the flow meter per liter
* 
* Every flow meter returns a different amount of impulses per liter. Set the correct value here.\n
* This parameter is used by the timer_getLH function to calculate the correct value.
*/
void timer_setImpulsePerLiter(uint16_t aValue);

/**
* @brief Get liters per hour of the flow meter
* @return uint16_t the flow in liters per hour
*
* Is it necessary to specify the correct impulses per liter.
*/
uint16_t timer_getLH(void);

/**
* @brief Timer tick, each 524 ms
* @return uint8_t Returns 1 one time every 524 ms
*/
uint8_t timer_524ms(void);

/**
* @brief Timer tick, each 100 ms
* @return uint8_t Returns 1 one time every 100 ms
*/
uint8_t timer_100ms(void);

#endif /* TIMER_H_ */