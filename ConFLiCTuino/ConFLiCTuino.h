/*
 * ConFLiCTuino.h
 *
 * Created: 06.03.2014 18:03:04
 *  Author: Robert
 */ 


#ifndef CONFLICTUINO_H_
#define CONFLICTUINO_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#define NUMBEROFCHANNELS 4

#define FIRMWAREVERSION 5

enum Stati{off,startup,on};


/**
 * @brief CHANNELDATA contains the permanent data of a channel.
 */
typedef struct{						
	uint8_t minTemp[24];						/**< minimum temperatures */
	uint8_t maxTemp[24];						/**< maximum temperatures */
	uint8_t manualPower;						/**< the power in manual mode */
	uint8_t minimumPower;						/**< the minimum power in automatic mode */
	uint8_t startupTime;						/**< time [s] = startupTime * 0.524 */
	uint8_t automaticMode;						/**< if 1 automatic mode enabled */
	uint8_t stopEnable;							/**< if 1 fan is able to stop in automatic mode */
	uint8_t threshold;							/**< a threshold value in % for the start in automatic mode */
}CHANNELDATA;									// 54 bytes


/**
 * @brief CHANNELSTATUS contains the status (non permanent data) of a channel.
 */
typedef struct{						
	uint16_t rpm;								/**< rotation per minute */
	uint8_t power;								/**< the actual power of the channel in % */
	uint8_t startupTimer;						/**< the timer witch counts the startup time down */
	enum Stati status;							/**< the status of the channel */	 
}CHANNELSTATUS;									// 5 bytes


/**
 * @brief LEDDATA contains the data of a led channel.
 */
typedef struct{						
	uint8_t mode;								/**< mode of leds */
	uint8_t manualPower[3];						/**< the power in manual mode */ 
}LEDDATA;										// 4 bytes


/**
 * @brief ALERTDATA contains the enable switches for all alerts.
 */
typedef struct{						
	uint8_t overtempEnable;						/**< 1 if over temperture alert enabled */
	uint8_t fanblockEnable;						/**< 1 if fan block alert enabled */
	uint16_t minWaterFlow;						/**< water flow minimum value for alert */ 
}ALERTDATA;										// 4 bytes


/**
 * @brief ALERTSTATUS contains the status of all alerts.
 */
typedef struct{						
	uint8_t overtemp;							/**< 1 if over temperture alert */
	uint8_t fanblock;							/**< 1 if a fan is blocked */
	uint16_t lowWaterFlow;						/**< 1 if water flow is under minimum value */ 
}ALERTSTATUS;		


/**
 * @brief ALPHANUMERIC contains the data for the alphanumeric display.
 */
typedef struct{						
	uint8_t backlight;							/**< backlight brightness in % */
	uint8_t contrast;							/**< contrast in % */
	uint8_t screen[31];							/**< screen time[s] = screen[i] * 0.524 */
	uint8_t content[255];						/**< the content */ 
}ALPHANUMERIC;									// 288 bytes



#endif /* CONFLICTUINO_H_ */