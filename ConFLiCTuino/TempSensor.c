/*********************************************************************/
/**
 * @file TempSensor.c
 * @brief Temperature Sensors depended functions include one wire
 * @author Robert Steigemann
 *
 * project: ConFLiCT
 *
 * Special thanks to Martin Thomas (eversmith@heizung-thomas.de)
 * and Peter Dannegger (danni@specs.de) for their One Wire Code.
 * This file use modified parts of this open source code downloaded at:
 * http://siwawi.bauing.uni-kl.de/avr_projects/tempsensor/?.
 *********************************************************************/

#include "TempSensor.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>

// table for temperature conversation of analog NTC sensors
const uint8_t tempTable[512] PROGMEM = {199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	199,
	198,
	195,
	193,
	190,
	188,
	186,
	184,
	182,
	180,
	178,
	176,
	174,
	173,
	171,
	169,
	168,
	166,
	165,
	163,
	162,
	160,
	159,
	157,
	156,
	155,
	154,
	152,
	151,
	150,
	149,
	148,
	146,
	145,
	144,
	143,
	142,
	141,
	140,
	139,
	138,
	137,
	136,
	135,
	134,
	133,
	132,
	132,
	131,
	130,
	129,
	128,
	127,
	126,
	126,
	125,
	124,
	123,
	123,
	122,
	121,
	120,
	120,
	119,
	118,
	118,
	117,
	116,
	115,
	115,
	114,
	113,
	113,
	112,
	112,
	111,
	110,
	110,
	109,
	109,
	108,
	107,
	107,
	106,
	106,
	105,
	104,
	104,
	103,
	103,
	102,
	102,
	101,
	101,
	100,
	100,
	99,
	99,
	98,
	98,
	97,
	97,
	96,
	96,
	95,
	95,
	94,
	94,
	93,
	93,
	92,
	92,
	91,
	91,
	91,
	90,
	90,
	89,
	89,
	88,
	88,
	88,
	87,
	87,
	86,
	86,
	85,
	85,
	85,
	84,
	84,
	83,
	83,
	83,
	82,
	82,
	82,
	81,
	81,
	80,
	80,
	80,
	79,
	79,
	79,
	78,
	78,
	77,
	77,
	77,
	76,
	76,
	76,
	75,
	75,
	75,
	74,
	74,
	74,
	73,
	73,
	73,
	72,
	72,
	72,
	71,
	71,
	71,
	70,
	70,
	70,
	69,
	69,
	69,
	68,
	68,
	68,
	67,
	67,
	67,
	67,
	66,
	66,
	66,
	65,
	65,
	65,
	64,
	64,
	64,
	64,
	63,
	63,
	63,
	62,
	62,
	62,
	62,
	61,
	61,
	61,
	60,
	60,
	60,
	60,
	59,
	59,
	59,
	59,
	58,
	58,
	58,
	57,
	57,
	57,
	57,
	56,
	56,
	56,
	56,
	55,
	55,
	55,
	55,
	54,
	54,
	54,
	54,
	53,
	53,
	53,
	53,
	52,
	52,
	52,
	52,
	51,
	51,
	51,
	51,
	50,
	50,
	50,
	50,
	49,
	49,
	49,
	49,
	48,
	48,
	48,
	48,
	47,
	47,
	47,
	47,
	47,
	46,
	46,
	46,
	46,
	45,
	45,
	45,
	45,
	44,
	44,
	44,
	44,
	44,
	43,
	43,
	43,
	43,
	42,
	42,
	42,
	42,
	42,
	41,
	41,
	41,
	41,
	41,
	40,
	40,
	40,
	40,
	39,
	39,
	39,
	39,
	39,
	38,
	38,
	38,
	38,
	38,
	37,
	37,
	37,
	37,
	37,
	36,
	36,
	36,
	36,
	35,
	35,
	35,
	35,
	35,
	34,
	34,
	34,
	34,
	34,
	33,
	33,
	33,
	33,
	33,
	32,
	32,
	32,
	32,
	32,
	32,
	31,
	31,
	31,
	31,
	31,
	30,
	30,
	30,
	30,
	30,
	29,
	29,
	29,
	29,
	29,
	28,
	28,
	28,
	28,
	28,
	27,
	27,
	27,
	27,
	27,
	27,
	26,
	26,
	26,
	26,
	26,
	25,
	25,
	25,
	25,
	25,
	25,
	24,
	24,
	24,
	24,
	24,
	23,
	23,
	23,
	23,
	23,
	23,
	22,
	22,
	22,
	22,
	22,
	21,
	21,
	21,
	21,
	21,
	21,
	20,
	20,
	20,
	20,
	20,
	19,
	19,
	19,
	19,
	19,
	19,
	18,
	18,
	18,
	18,
	18,
	18,
	17,
	17,
	17,
	17,
	17,
	17,
	16,
	16,
	16,
	16,
	16,
	15,
	15,
	15,
	15,
	15,
	15,
	14,
	14,
	14,
	14,
	14,
	14,
	13,
	13,
	13,
	13,
	13,
	13,
	12,
	12,
	12,
	12,
	12,
	12,
	11,
	11,
	11,
	11,
	11,
	11,
	10,
	10,
	10,
	10,
	10,
	10,
	9,
	9,
	9,
	9,
	9,
	9,
	8,
	8,
	8,
	8,
	8,
	8,
	7,
	7,
	7,
	7,
	7,
	7,
	6,
	6,
	6,
	6,
	6,
	6,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	4};


uint8_t oneWireAddr[MAXSENSORS][8];		// the addresses of the one wire sensors
uint8_t oneWireAmount = 0;				// the number of connected one wire sensors after initialization
uint8_t oneWireTemperatures[MAXSENSORS] = {40,40,40,40,40,40,40,40};


// defines for one wire pin
#define OW_GET_IN()   ( OW_IN & (1<<OW_PIN))
#define OW_OUT_LOW()  ( OW_OUT &= (~(1 << OW_PIN)) )
#define OW_OUT_HIGH() ( OW_OUT |= (1 << OW_PIN) )
#define OW_DIR_IN()   ( OW_DDR &= (~(1 << OW_PIN )) )
#define OW_DIR_OUT()  ( OW_DDR |= (1 << OW_PIN) )


// check the one wire pin returns 0 if low
uint8_t ow_input_pin_state()
{
	return OW_GET_IN();
}

// make a reset impulse on the one wire pin
uint8_t ow_reset(void)
{
	uint8_t err;
	
	OW_OUT_LOW();
	OW_DIR_OUT();            // pull OW-Pin low for 480us
	_delay_us(482);

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// set Pin as input - wait for clients to pull low
		OW_DIR_IN(); // input
				
		_delay_us(64);       // was 66
		err = OW_GET_IN();   // no presence detect if err!=0: nobody pulled to low, still high
	}
	
	// after a delay the clients should release the line
	// and input-pin gets back to high by pull-up-resistor
	_delay_us(482 - 64);       // was 480-66
	if( OW_GET_IN() == 0 ) {
		err = 1;             // short circuit, expected high but got low
	}	
	return err;
}

// transfer one bit of the one wire pin
static uint8_t ow_bit_io( uint8_t b)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		OW_DIR_OUT();    // drive bus low
		_delay_us(2);    // T_INT > 1usec accoding to timing-diagramm
		if ( b )
		{
			OW_DIR_IN(); // to write "1" release bus, resistor pulls high
		}

		// "Output data from the DS18B20 is valid for 15usec after the falling
		// edge that initiated the read time slot. Therefore, the master must 
		// release the bus and then sample the bus state within 15ussec from 
		// the start of the slot."
		_delay_us(15-2-OW_CONF_DELAYOFFSET);
		
		if( OW_GET_IN() == 0 ) {
			b = 0;  // sample at end of read-timeslot
		}
	
		_delay_us(60-15-2+OW_CONF_DELAYOFFSET);

		OW_DIR_IN();
	
	
	} /* ATOMIC_BLOCK */

	_delay_us(OW_RECOVERY_TIME); // may be increased for longer wires

	return b;
}

// write one byte
uint8_t ow_byte_wr( uint8_t b )
{
	uint8_t i = 8, j;
	
	do {
		j = ow_bit_io( b & 1 );
		b >>= 1;
		if( j ) {
			b |= 0x80;
		}
	} while( --i );
	
	return b;
}

// read one byte
uint8_t ow_byte_rd( void )
{
	// read by sending only "1"s, so bus gets released
	// after the init low-pulse in every slot
	return ow_byte_wr( 0xFF );
}

// search for (next) device
uint8_t ow_rom_search( uint8_t diff, uint8_t *id )
{
	uint8_t i, j, next_diff;
	uint8_t b;
	
	if( ow_reset() ) {
		return OW_PRESENCE_ERR;         // error, no device found <--- early exit!
	}
	
	ow_byte_wr( OW_SEARCH_ROM );        // ROM search command
	next_diff = OW_LAST_DEVICE;         // unchanged on last device
	
	i = OW_ROMCODE_SIZE * 8;            // 8 bytes
	
	do {
		j = 8;                          // 8 bits
		do {
			b = ow_bit_io( 1 );         // read bit
			if( ow_bit_io( 1 ) ) {      // read complement bit
				if( b ) {               // 0b11
					return OW_DATA_ERR; // data error <--- early exit!
				}
			}
			else {
				if( !b ) {              // 0b00 = 2 devices
					if( diff > i || ((*id & 1) && diff != i) ) {
						b = 1;          // now 1
						next_diff = i;  // next pass 0
					}
				}
			}
			ow_bit_io( b );             // write bit
			*id >>= 1;
			if( b ) {
				*id |= 0x80;            // store bit
			}
			
			i--;
			
		} while( --j );
		
		id++;                           // next byte
		
	} while( i );
	
	return next_diff;                   // to continue search
}

// send command to a special client or to all clients (id = NULL)
static void ow_command( uint8_t command, uint8_t *id)
{
	uint8_t i;

	ow_reset();

	if( id ) {
		ow_byte_wr( OW_MATCH_ROM );     // to a single device
		i = OW_ROMCODE_SIZE;
		do {
			ow_byte_wr( *id );
			id++;
		} while( --i );
	}
	else {
		ow_byte_wr( OW_SKIP_ROM );      // to all devices
	}
	ow_byte_wr( command );
}



void tempSensor_init(void)
{
	DIDR0 = 0xFF;						// disable digital input buffers
	ADMUX = 0b11000000;					// 2.56 V reference, right adjust 
	ADCSRB = 0;							// no gain
	ADCSRA = 0b11000111;				// enable ADC, start first conversation to calibrate, prescaler = 128 => 125 kHz
	
	
	// init 1-Wire here
	
	volatile uint8_t diff;
	
	// find sensors and store adresses
	
	diff = OW_SEARCH_FIRST;
	while ( diff != OW_LAST_DEVICE && oneWireAmount < MAXSENSORS ) {
		diff = ow_rom_search(diff,&oneWireAddr[oneWireAmount][0]);
		if (diff == OW_PRESENCE_ERR || diff == OW_DATA_ERR)
		{
			break;
		}
		oneWireAmount++;
	}
	if (oneWireAmount > 0)
	{
		ow_command(DS18X20_CONVERT_T, NULL);	// start conversation of all connected 1-wire sensors
	}
}


uint8_t tempSensor_getTemp(uint8_t channel)
{		
	uint8_t i;
		
	if (oneWireAmount > 0)		// interface the 1-wire sensors if connected
	{
		if (ow_bit_io(1))		// conversation finished
		{
			for (i=0;i<oneWireAmount;i++)			// for all connected 1-wire sensors
			{
				ow_command(DS18X20_READ, &oneWireAddr[i][0]);		// start reading scratchpad
			
				oneWireTemperatures[i] = ow_byte_rd();	// read only the lsb of the temperature
			
				// ow_reset();							// abort conversation (already in ow_command included)
			
				if (oneWireTemperatures[i] > 199)					// check for maximum of 99.5°C
				{
					oneWireTemperatures[i] = 199;
				}
			}
			ow_command(DS18X20_CONVERT_T, NULL);	// start conversation of all connected 1-wire sensors		
		}
	}		
	
	
	if (channel < 8)	// analog sensor
	{
		uint16_t value;
		
		ADMUX = (ADMUX & 0b11111000)|channel;	// select channel
		
		ADCSRA |= 0b01000000;					// start conversaition
		
		while(ADCSRA & 0b01000000);
				
		value = ADC;
		
		if (value == 1023)
		{
			return 220;
		}
		else
		{
			value = (value >> 1);
			return pgm_read_byte(tempTable+value);
		}
	}
	else				// 1-wire sensor
	{
		if (channel < (8+oneWireAmount))	// sensor is connected
		{
			return oneWireTemperatures[channel-8];	// latest value out of memory			
		} 
		else
		{
			return 220;
		}		
	}	
}


uint8_t tempSensor_getOneWireAmount(void)
{
	return oneWireAmount;
}


uint8_t tempSensor_getOneWireID(uint8_t index)
{
	return *(&oneWireAddr[0][0]+index);	
}