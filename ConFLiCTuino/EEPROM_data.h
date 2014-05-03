/*
 * EEPROM_data.h
 *
 * Created: 03.02.2014 20:47:29
 *  Author: Robert
 */ 


#ifndef EEPROM_DATA_H_
#define EEPROM_DATA_H_

// Mark of the EEPROMVERSION, it is independent of the Firmware Version.
// Only increment it, when changes of the existing data format was made.
#define EEPROMVERSION 1


#define EEPROMVERSION_BITS (0x0100 * EEPROMVERSION + EEPROMVERSION)


// Default values, change it here.

#define DEFAULT_CHANNEL_DATA_VALUES {																			\
	{250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250},			\
	{255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},			\
	100,33,6,0,0,10}

#define DEFAULT_SPARE_TEMPERATURE_VALUES {160,160,160,160,160,160,160,160}
	
#define DEFAULT_LED_DATA_VALUES {0,{100,100,100}}

#define DEFAULT_ALERT_DATA_VALUES {0,0,0}
	
#define DEFAULT_PULSEPERLITER_VALUE 169

#define DEFAULT_ALPHADISLAY_DATA {																				\
	100, 50,																									\
	{13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},											\
	{'1',':',' ',197,1,0,0,'R','P','M',' ',' ',198,1,0,0x25,													\
		'2',':',' ',197,2,0,0,'R','P','M',' ',' ',198,2,0,0x25,													\
		'3',':',' ',197,3,0,0,'R','P','M',' ',' ',198,3,0,0x25,													\
		192,1,0,0,' ',0xF2,'C',' ',192,4,0,0,' ',0xF2,'C',' ',													\
		192,2,0,0,' ',0xF2,'C',' ',192,5,0,0,' ',0xF2,'C',' ',													\
		192,3,0,0,' ',0xF2,'C',' ',192,6,0,0,' ',0xF2,'C',' ',													\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,						\
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20}									\
}


// Mapping default data to eeprom to generate the .eep file.
// The .eep file is not used when flashing the arduino via the XLoder tool (general via bootloader).
// It is only for extra programming tools and for debugging/comparing reasons.

uint16_t EEMEM eeprom_eepromVersionChecker = EEPROMVERSION_BITS;

CHANNELDATA EEMEM eeprom_channel_data[NUMBEROFCHANNELS] = {DEFAULT_CHANNEL_DATA_VALUES, DEFAULT_CHANNEL_DATA_VALUES, DEFAULT_CHANNEL_DATA_VALUES, DEFAULT_CHANNEL_DATA_VALUES};

uint8_t EEMEM eeprom_spare_temperature[8] = DEFAULT_SPARE_TEMPERATURE_VALUES;

LEDDATA EEMEM eeprom_led_data = DEFAULT_LED_DATA_VALUES;

ALERTDATA EEMEM	eeprom_alert_data = DEFAULT_ALERT_DATA_VALUES;

uint16_t EEMEM eeprom_pulsePerLiter = DEFAULT_PULSEPERLITER_VALUE;

ALPHANUMERIC EEMEM eeprom_alphaDisplay = DEFAULT_ALPHADISLAY_DATA;


// Mapping default data to flash memory. If the arduino starts first time or if the EEPROMVERSION has increased by a firmware update,
// the firmware copy the values form flash to eeprom. This resets the eeprom to the default values.

const CHANNELDATA PROGMEM default_channel_data = DEFAULT_CHANNEL_DATA_VALUES;

const uint8_t PROGMEM default_spare_temperature[8] = DEFAULT_SPARE_TEMPERATURE_VALUES;

const LEDDATA PROGMEM default_led_data = DEFAULT_LED_DATA_VALUES;

const ALERTDATA	PROGMEM default_alert_data = DEFAULT_ALERT_DATA_VALUES;

const uint16_t PROGMEM default_pulsePerLiter = DEFAULT_PULSEPERLITER_VALUE;

const ALPHANUMERIC PROGMEM default_alphaDisplay = DEFAULT_ALPHADISLAY_DATA;


#endif /* EEPROM_DATA_H_ */