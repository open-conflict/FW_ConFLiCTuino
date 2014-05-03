/*********************************************************************/
/**
 * @file TempSensor.h
 * @brief Temperature Sensors depended functions include one wire
 * @author Robert Steigemann
 *
 * project: ConFLiCT
 *
 * Special thanks to Martin Thomas (eversmith@heizung-thomas.de)
 * and Peter Dannegger (danni@specs.de) for their one wire Code.
 * This file use modified parts of this open source code downloaded at:
 * http://siwawi.bauing.uni-kl.de/avr_projects/tempsensor/?.
 *
 * @date 12.01.2014 - first implementation
 * @date 23.02.2014 - one wire included
 *********************************************************************/

#include <avr/io.h>

#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_

#define MAXSENSORS 8	// DS18X20 sensors
// Recovery time (T_Rec) minimum 1usec - increase for long lines
// 5 usecs is a value give in some Maxim AppNotes
// 30u secs seem to be reliable for longer lines
//#define OW_RECOVERY_TIME        5  /* usec */
//#define OW_RECOVERY_TIME      300 /* usec */
#define OW_RECOVERY_TIME         10 /* usec */
#define OW_MATCH_ROM    0x55
#define OW_SKIP_ROM     0xCC
#define OW_SEARCH_ROM   0xF0

#define OW_SEARCH_FIRST 0xFF        // start new search
#define OW_PRESENCE_ERR 0xFF
#define OW_DATA_ERR     0xFE
#define OW_LAST_DEVICE  0x00        // last device found

#define OW_CONF_DELAYOFFSET 0

// rom-code size including CRC
#define OW_ROMCODE_SIZE 8

#define DS18X20_CONVERT_T         0x44
#define DS18X20_READ              0xBE

// !!!define used one wire pin here!!!
#define OW_PIN  3
#define OW_IN   PINE
#define OW_OUT  PORTE
#define OW_DDR  DDRE


/**
* @brief Initialize the temperature sensors.
*
* Do this before using an other function.\n
*/
void tempSensor_init(void);

/**
* @brief Get the temperature of a sensor.
* @param channel The channel to get the temperature from (0-15).
* @return uint8_t The temperature in (°C x 2) (0-199 = 0-99.5°C),\n
*                 220 if sensor is not connected. 
*/
uint8_t tempSensor_getTemp(uint8_t channel);

/**
* @brief Get the amount of connected one wire sensors.
* @return uint8_t The amount of connected one wire sensors (0-8).
*/
uint8_t tempSensor_getOneWireAmount(void);

/**
* @brief Get a part of an ID of a one wire sensor.
* @param index The index of the part of the ID (0-63).
* @return uint8_t One byte of a one wire ID.
*
* Each connected sensor has his own ID. Each ID is 8 bytes long.\n
* The IDs are organized one after each other. So the returned bytes
* of index 0-7 contains the ID of the first sensor. The ID of the second
* sensor are returned by index 8-15 and so on. If you ask for an ID of a
* sensor, which is not connected, the returned bytes are not specified.
*/
uint8_t tempSensor_getOneWireID(uint8_t index);


#endif /* TEMPSENSOR_H_ */