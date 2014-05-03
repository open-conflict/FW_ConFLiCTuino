/*
 * ConFLiCTuino.c
 *
 * Created: 03.05.2014
 *  Author: Robert
 *
 * FW C 0.5
 *
 * First Github version
 *
 * 
 *
 */ 


#include "ConFLiCTuino.h"
#include "Portconfig.h"
#include "Serialport.h"
#include "Timer.h"
#include "TempSensor.h"
#include "EEPROM_data.h"


int main(void)
{
    port_init();
	serialport_init();
	timer_init();
	tempSensor_init();
	//alpha_init();
	
	// activate interrupts
	sei();
	
	uint8_t i,j;						// multi used index variables
	uint16_t k;
	
	uint8_t messageCtr = 0;				// message counter for automatic messages 
	
	CARRIAGE aDataSet;					// a data set used for communication
	
	uint8_t temperature[24];			// the temperatures, actual only 0-7 are in use
	uint8_t spare_temperature[8];		// the spare values for the aida values (actual not in use)
	uint8_t spare_temperatureTimer = 5; // timeout timer for AIDA temperature (use spare values until values are older then 4 seconds)
	
	uint16_t literPerHour = 0;
		
	CHANNELDATA channel_data[NUMBEROFCHANNELS];		// the permanent data of the 4 channels (fan 1-3 and analog)
	CHANNELSTATUS channel_status[NUMBEROFCHANNELS];	// the actual status of the channels
	
	LEDDATA led_data;					// data for the led channels
	uint16_t ledCtr = 0;				// a rotary counter value for led sweep calculation, incremented every 100 ms
	
	ALERTDATA alert_data;				// data for alert
	ALERTSTATUS alert_status;			// actual status
	
	uint8_t time[3] = {0,0,0};			// = {h,m,s}
	uint16_t pcStatus[4] = {0,0,0,0};	// = {CPU_CLK,CPU%,GPU%,RAM%}
	
	uint16_t pulsePerLiter;
	
	
	if (eeprom_read_word(&eeprom_eepromVersionChecker) != EEPROMVERSION_BITS)			// EEPROM is not OK
	{		
		
		// serialport_printf("#EEPROM FAILED, START TO OVERWRITE IT!#\r");
		
		for (j=0;j<NUMBEROFCHANNELS;j++)
		{
			for (i=0;i<sizeof(CHANNELDATA);i++)
			{
				eeprom_write_byte(((uint8_t *)&eeprom_channel_data[j])+i,pgm_read_byte(((uint8_t *)&default_channel_data)+i));
			}			
		}
		
		// serialport_printf("#CHANNELS COMPLETE#\r");
				
		for (i=0;i<8;i++)
		{
			eeprom_write_byte((&eeprom_spare_temperature[i]),pgm_read_byte(&default_spare_temperature[i]));
		}
		
		// serialport_printf("#SPARE TEMPERATURES COMPLETE#\r");
		
		for (i=0;i<sizeof(LEDDATA);i++)
		{
			eeprom_write_byte(((uint8_t *)&eeprom_led_data)+i,pgm_read_byte(((uint8_t *)&default_led_data)+i));
		}
		
		// serialport_printf("#LED COMPLETE#\r");
		
		for (i=0;i<sizeof(ALERTDATA);i++)
		{
			eeprom_write_byte(((uint8_t *)&eeprom_alert_data)+i,pgm_read_byte(((uint8_t *)&default_alert_data)+i));
		}
		
		// serialport_printf("#ALERT COMPLETE#\r");
		
		eeprom_write_word(&eeprom_pulsePerLiter,pgm_read_word(&default_pulsePerLiter));
		
		// serialport_printf("#PULSEPERLITER COMPLETE#\r");
		
		for (k=0;k<sizeof(ALPHANUMERIC);k++)
		{
			eeprom_write_byte(((uint8_t *)&eeprom_alphaDisplay)+k,pgm_read_byte(((uint8_t *)&default_alphaDisplay)+k));
		}
		
		// serialport_printf("\r#DISPLAY COMPLETE#\r");
		
		eeprom_write_word(&eeprom_eepromVersionChecker,EEPROMVERSION_BITS);
		
		// serialport_printf("#EEPROMVERSION COMPLETE#\r");
		// serialport_printf("#EEPROM OVERRIDE COMPLETE, START#\r");
	}
	
	
	pulsePerLiter = eeprom_read_word(&eeprom_pulsePerLiter);	// impulses per liter of the flow meter
	timer_setImpulsePerLiter(pulsePerLiter);							// inform the timer
	
	//ALPHANUMERIC alphaDisplay;
	//uint8_t screenIndex = 0;
	//uint8_t screenCountdown;
	//uint8_t screenOverlayMessageCountdown = 0;
	//
	//uint8_t switch1_ctr = 0;
	//uint8_t switch2_ctr = 0;
	//
	//uint8_t buzzerEnable = 1;
	
	// read the EEPROM
	eeprom_read_block((void*)channel_data, (const void*)eeprom_channel_data, (NUMBEROFCHANNELS*sizeof(CHANNELDATA)));
	eeprom_read_block((void*)spare_temperature, (const void*)eeprom_spare_temperature, 8);
	eeprom_read_block((void*)&led_data, (const void*)&eeprom_led_data, sizeof(LEDDATA));
	eeprom_read_block((void*)&alert_data, (const void*)&eeprom_alert_data, sizeof(ALERTDATA));
	//eeprom_read_block((void*)&alphaDisplay, (const void*)&eeprom_alphaDisplay, sizeof(ALPHANUMERIC));
	//timer_setPwm(BACKLIGHT,alphaDisplay.backlight);
	//alpha_setContrast(alphaDisplay.contrast);
	//screenCountdown = alphaDisplay.screen[0];	
	
	
	for (i=0;i<NUMBEROFCHANNELS;i++)
	{
		channel_status[i].startupTimer = channel_data[i].startupTime;
		channel_status[i].status = startup;		
	}
	
		
	
	
	//#ifdef REV2_1
		//alpha_displayOverlayMessage("    ConFLiCT                    FW: C0.3 HW: 2.1");
	//#endif
	//#ifdef REV2_2
		//alpha_displayOverlayMessage("    ConFLiCT                    FW: C0.3 HW: 2.2"); 
	//#endif	
	//screenOverlayMessageCountdown = 10; 
		
	while(1)						// main loop
    {
        		
		// use spare temperatures if aida temperatures are to old
		if (spare_temperatureTimer > 4)
		{
			for (i=0;i<8;i++)
			{
				temperature[i+16] = spare_temperature[i];
			}
		}
				
		alert_status.overtemp = 0;				// reset the overtemp alert 
		
		// calculate the channel power
		for (i=0;i<NUMBEROFCHANNELS;i++)
		{	
					
			if (channel_data[i].automaticMode)		// automatic mode
			{
				uint8_t cooling = 0;			// temporary cooling value
				uint8_t d1, d2;					// temporary variable for calculation
				for (j=0;j<24;j++)
				{
					if ((temperature[j] >= channel_data[i].minTemp[j]) && (temperature[j] <= channel_data[i].maxTemp[j])) // temperature is between min and max
					{
						d1 = 100 / (channel_data[i].maxTemp[j] - channel_data[i].minTemp[j]);	// calculate cooling value
						d2 = (temperature[j] - channel_data[i].minTemp[j]) * d1;
						if (d2 > cooling)	// save the highest cooling value	
						{
							cooling = d2;
						}						
					}
					if (temperature[j] > channel_data[i].maxTemp[j])	// a temperature over maximum! o.O
					{
						// over temperature
						if (alert_data.overtempEnable)
						{
							alert_status.overtemp = 1;
						}					
						cooling = 100;
					}				
				}
				
				switch (channel_status[i].status)						// check the status
				{
					case off:		
						channel_status[i].power = 0;					// turn it off
						if (channel_data[i].stopEnable)					// off allowed
						{
							if (cooling > channel_data[i].threshold)	// if cooling is higher then threshold
							{
								channel_status[i].status = startup;		// start the channel and reset the startup timer
								channel_status[i].startupTimer = channel_data[i].startupTime;
							}							
						}
						else                                            // off is not allowed (anymore)
						{
							channel_status[i].status = startup;			// start the channel and reset the startup timer
							channel_status[i].startupTimer = channel_data[i].startupTime;
						}
						break;
					
					case startup:										
						channel_status[i].power = 100;					// 100 % for startup
						break;
					
					case on:
						if ((cooling == 0) && (channel_data[i].stopEnable == 1))	// if stop is enabled and no cooling needed
						{
							channel_status[i].status = off;				// turn it off
						}
						// calculate the power value out off the minimum value and the cooling value
						channel_status[i].power = (uint8_t)((((uint16_t)(100-channel_data[i].minimumPower))*cooling)/100 + channel_data[i].minimumPower);
						break;
				}				
			}
			 
			else								// manual mode
			{
				switch (channel_status[i].status)					// check the status
				{
					case off:						
						channel_status[i].power = 0;				// turn it off
						if (channel_data[i].manualPower > 0)		
						{
							channel_status[i].status = startup;		// start the channel and reset the startup timer
							channel_status[i].startupTimer = channel_data[i].startupTime;
						}					
						break;
					
					case startup:
						channel_status[i].power = 100;				// 100 % for startup
						break;
						
					case on:
						channel_status[i].power = channel_data[i].manualPower;
						if (channel_data[i].manualPower == 0)		// if no power, go to off
						{
							channel_status[i].status = off;
						}
						break;						
				}						
			}	
					
		}
		
		alert_status.fanblock = 0;		// reset the fan block status
		
		// exchange channel with hardware timer
		for (i=0;i<NUMBEROFCHANNELS;i++)
		{
			timer_setPwm(i,channel_status[i].power);
			channel_status[i].rpm = timer_getRpm(i);
			if (channel_status[i].status == on && channel_status[i].rpm == 0 && alert_data.fanblockEnable && i != 3)
			{
				alert_status.fanblock = 1;		// fan is blocked or not started correctly
			}			
		}
		
		// get liter per hour
		literPerHour = timer_getLH();
		if (literPerHour < alert_data.minWaterFlow)
		{
			alert_status.lowWaterFlow = 1;
		}
		else
		{
			alert_status.lowWaterFlow = 0;
		}
			
		
		
		// led exchange and calculating
		switch (led_data.mode)		
		{
			case 0:		// manual mode
				timer_setPwm(LED_RED,led_data.manualPower[0]);
				timer_setPwm(LED_GREEN,led_data.manualPower[1]);
				timer_setPwm(LED_BLUE,led_data.manualPower[2]);
				break;
				
			case 1:
				// Do calculation with ledCtr, timer_setPWM(LED_X,value), and local variables only!
				if (ledCtr < 100)
				{
					timer_setPwm(LED_RED,ledCtr);
					timer_setPwm(LED_GREEN,0);
					timer_setPwm(LED_BLUE,0);
				} 
				else if(ledCtr < 200)
				{
					timer_setPwm(LED_RED,200-ledCtr);
					timer_setPwm(LED_GREEN,0);
					timer_setPwm(LED_BLUE,0);
				}
				else if (ledCtr < 300)
				{
					timer_setPwm(LED_GREEN,ledCtr-200);
					timer_setPwm(LED_RED,0);
					timer_setPwm(LED_BLUE,0);
				}
				else if (ledCtr < 400)
				{
					timer_setPwm(LED_GREEN,400-ledCtr);
					timer_setPwm(LED_RED,0);
					timer_setPwm(LED_BLUE,0);
				}
				else if (ledCtr < 500)
				{
					timer_setPwm(LED_BLUE,ledCtr-400);
					timer_setPwm(LED_RED,0);
					timer_setPwm(LED_GREEN,0);
				}
				else if (ledCtr < 600)
				{
					timer_setPwm(LED_BLUE,600-ledCtr);
					timer_setPwm(LED_RED,0);
					timer_setPwm(LED_GREEN,0);
				} 
				else
				{
					ledCtr = 0;
				}
				break;
				
			default:	// wrong mode, red led on
				timer_setPwm(LED_BLUE,0);
				timer_setPwm(LED_GREEN,0);
				timer_setPwm(LED_RED,100);
				break;				
		} 
		
				
			
		
		// each 524ms
		if (timer_524ms())
		{			
			// decrement the channel startup timers
			for (i=0;i<NUMBEROFCHANNELS;i++)
			{
				if (channel_status[i].startupTimer > 1)
				{
					channel_status[i].startupTimer--;
				}
				if (channel_status[i].startupTimer == 1)
				{
					channel_status[i].startupTimer = 0;
					channel_status[i].status = on;		// go to on if startup time is over
				}
			}
			
			// increment the spare temperature timer
			if (spare_temperatureTimer < 255)
			{
				spare_temperatureTimer ++;
			}
			
			// beeper
			//if ((alert_status.fanblock || alert_status.lowWaterFlow || alert_status.overtemp) && buzzerEnable)
			//{
				//// toggle beeper
				//PORTR_OUTTGL = 1;
			//} 
			//else
			//{
				//// turn beeper off
				//PORTR_OUTCLR = 1;
			//}
			
			
			// display
			//screenCountdown --;						
			//if (screenCountdown == 0)
			//{
				//screenIndex ++;
				//if (alphaDisplay.screen[screenIndex] == 0)		// screen is not available
				//{
					//screenIndex = 0;		// go to first screen					
				//} 
				//screenCountdown = alphaDisplay.screen[screenIndex];				
			//}
			//if (screenOverlayMessageCountdown == 0)
			//{
				//// if no alert is active, show normal display content
				//if (alert_status.fanblock == 0 && alert_status.lowWaterFlow == 0 && alert_status.overtemp == 0)
				//{
					//alpha_updateScreen(screenIndex,alphaDisplay.content,temperature,time,pcStatus,&literPerHour,channel_status);
				//}
				//else // else show the alert
				//{
					//alpha_displayAlert(alert_status);
				//}
			//} 
			//else
			//{
				//screenOverlayMessageCountdown--;
			//}
			
				
		}
			
		// each 100ms
		if (timer_100ms())
		{
			// get the temperatures
			for (i=0;i<16;i++)
			{
				temperature[i] = tempSensor_getTemp(i);		// read out up to 8 NTC-Sensors and 8 1-Wire sensors
			}
						
			
			ledCtr++;
			
			// check switch 1, left button
			//if (SWITCH1)
			//{
				//if (switch1_ctr < 255)
				//{
					//switch1_ctr++;
				//}
			//}
			//else
			//{
				//switch1_ctr = 0;
			//}
			//
			//if (switch1_ctr == 2)
			//{
				//if (buzzerEnable)
				//{
					//buzzerEnable = 0;
					//// turn beeper off
					//PORTR_OUTCLR = 1;
					//alpha_displayOverlayMessage("buzzer disabled");
				//}
				//else
				//{
					//buzzerEnable = 1;
					//alpha_displayOverlayMessage("buzzer enabled");
				//}
				//screenOverlayMessageCountdown = 2;				
			//}
			
			// check the next button (switch 2, right button)
			//if (SWITCH2)
			//{				
				//if (switch2_ctr < 255)
				//{					
					//switch2_ctr++;					
				//}		
			//}
			//else
			//{
				//switch2_ctr = 0;
			//}
			//
			//
			//// change display content if button 2 is short pressed or every 1 second pressed
			//if (switch2_ctr == 10)
			//{
				//switch2_ctr = 1;
			//}
			//
			//if (switch2_ctr == 1)
			//{
				//screenIndex ++;
				//if (alphaDisplay.screen[screenIndex] == 0)		// screen is not available
				//{
					//screenIndex = 0;		// go to first screen
				//}
				//screenCountdown = alphaDisplay.screen[screenIndex];
				//// if no alert is active, show normal display content
				//if (alert_status.fanblock == 0 && alert_status.lowWaterFlow == 0 && alert_status.overtemp == 0)
				//{
					//alpha_updateScreen(screenIndex,alphaDisplay.content,temperature,time,pcStatus,&literPerHour,channel_status);
				//}
			//}
			
			
			
			// send some actual information to the PC
			// every 100 ms an other of these messages 
			messageCtr++;
			aDataSet.dA = SERIALPORT_DATA;
			switch (messageCtr)
			{
				case 1:		// temperatures
					aDataSet.id = 30;					
					for (i=0;i<24;i++)
					{
						aDataSet.index = i + 1;
						aDataSet.data = temperature[i];
						serialport_writeCarriage(&aDataSet);
					}
					break;
			
				case 2:		// alerts
					aDataSet.id = 50;
					aDataSet.index = 1;
					aDataSet.data = alert_status.overtemp;
					serialport_writeCarriage(&aDataSet);
					aDataSet.index = 2;
					aDataSet.data = alert_status.fanblock;
					serialport_writeCarriage(&aDataSet);
					aDataSet.index = 3;
					aDataSet.data = alert_status.lowWaterFlow;
					serialport_writeCarriage(&aDataSet);
					break;
			
				case 3:		// water flow
					aDataSet.id = 60;
					aDataSet.index = 1;
					aDataSet.data = literPerHour;
					serialport_writeCarriage(&aDataSet);
					break;
			
				case 4:		// RPM of channels
					aDataSet.id = 61;
					for (i= 0;i<4;i++)
					{
						aDataSet.index = i + 1;
						aDataSet.data = channel_status[i].rpm;
						serialport_writeCarriage(&aDataSet);
					}
					break;
			
				case 5:		// power values
					aDataSet.id = 62;
					aDataSet.dA = SERIALPORT_DATA;
					for (i= 0;i<NUMBEROFCHANNELS;i++)
					{
						aDataSet.index = i + 1;
						aDataSet.data = channel_status[i].power;
						serialport_writeCarriage(&aDataSet);
					}
					messageCtr = 0;
					break;			
			}				
		}			
		
		
		// check for received carriages
		if (serialport_getLastCarriage(&aDataSet) == 0)								// new carriage received
		{
						
			if (aDataSet.dA == SERIALPORT_DATA)			// new data
			{
				switch(aDataSet.id)
				{
					case 01:							// reset
						if (aDataSet.index == 170 && aDataSet.data == 85)
						{
							cli();						// disable interrupts
							wdt_reset();				// reset via watchdog
							_delay_ms(1);				// wait until reset occurs
						}						
						break;
						
					case 30:							// temperatures from AIDA or other PC software
						temperature[aDataSet.index-1] = aDataSet.data;
						spare_temperatureTimer = 0;
						break;
					
					case 11:							// minimum temperatures channel 0
						channel_data[0].minTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[0].minTemp[aDataSet.index-1], channel_data[0].minTemp[aDataSet.index-1]);
						break;
					
					case 21:							// maximum temperatures channel 0
						channel_data[0].maxTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[0].maxTemp[aDataSet.index-1], channel_data[0].maxTemp[aDataSet.index-1]);
						break;
					
					case 12:							// minimum temperatures channel 1
						channel_data[1].minTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[1].minTemp[aDataSet.index-1], channel_data[1].minTemp[aDataSet.index-1]);
						break;
					
					case 22:							// maximum temperatures channel 1
						channel_data[1].maxTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[1].maxTemp[aDataSet.index-1], channel_data[1].maxTemp[aDataSet.index-1]);
						break;
						
					case 13:							// minimum temperatures channel 2
						channel_data[2].minTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[2].minTemp[aDataSet.index-1], channel_data[2].minTemp[aDataSet.index-1]);
						break;
					
					case 23:							// maximum temperatures channel 2
						channel_data[2].maxTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[2].maxTemp[aDataSet.index-1], channel_data[2].maxTemp[aDataSet.index-1]);
						break;	
						
					case 14:							// minimum temperatures channel 3
						channel_data[3].minTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[3].minTemp[aDataSet.index-1], channel_data[3].minTemp[aDataSet.index-1]);
						break;
					
					case 24:							// maximum temperatures channel 3
						channel_data[3].maxTemp[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[3].maxTemp[aDataSet.index-1], channel_data[3].maxTemp[aDataSet.index-1]);
						break;
						
					case 31:							// spare temperatures
						spare_temperature[aDataSet.index-17] = aDataSet.data;
						eeprom_update_byte(&eeprom_spare_temperature[aDataSet.index-17], spare_temperature[aDataSet.index-17]);
						break;	
						
					case 40:							// time
						time[aDataSet.index-1] = aDataSet.data;
						break;
					
					case 41:							// PC information
						pcStatus[aDataSet.index-1] = aDataSet.data;
						break;
					
					case 51:							// alert enable
						if (aDataSet.index == 1)
						{
							alert_data.overtempEnable = aDataSet.data;
							eeprom_update_byte(&eeprom_alert_data.overtempEnable, alert_data.overtempEnable);
						} 
						else if (aDataSet.index == 2)
						{
							alert_data.fanblockEnable = aDataSet.data;
							eeprom_update_byte(&eeprom_alert_data.fanblockEnable, alert_data.fanblockEnable);
						}
						break;
					
						
					case 70:							// Pulse per liter of flowMeter
						if (aDataSet.index == 1)
						{
							pulsePerLiter = aDataSet.data;
							timer_setImpulsePerLiter(pulsePerLiter);
							eeprom_update_word(&eeprom_pulsePerLiter, pulsePerLiter);
						}
						else if (aDataSet.index == 2)
						{
							alert_data.minWaterFlow = aDataSet.data;
							eeprom_update_word(&eeprom_alert_data.minWaterFlow, alert_data.minWaterFlow);
						}
						break;
					
					case 71:							// manual power
						channel_data[aDataSet.index-1].manualPower = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[aDataSet.index-1].manualPower, channel_data[aDataSet.index-1].manualPower);
						break;
						
					case 72:							// startup times
						if (aDataSet.data == 0)
						{
							channel_data[aDataSet.index-1].startupTime = 1;							
						} 
						else
						{
							channel_data[aDataSet.index-1].startupTime = aDataSet.data;
						}
						eeprom_update_byte(&eeprom_channel_data[aDataSet.index-1].startupTime, channel_data[aDataSet.index-1].startupTime);
						break;
					
					case 73:							// minimum power
						channel_data[aDataSet.index-1].minimumPower = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[aDataSet.index-1].minimumPower, channel_data[aDataSet.index-1].minimumPower);
						break;
						
					case 74:							// automatic mode
						channel_data[aDataSet.index-1].automaticMode = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[aDataSet.index-1].automaticMode, channel_data[aDataSet.index-1].automaticMode);
						break;
						
					case 75:							// stop enable
						channel_data[aDataSet.index-1].stopEnable = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[aDataSet.index-1].stopEnable, channel_data[aDataSet.index-1].stopEnable);
						break;
						
					case 76:							// threshold
						channel_data[aDataSet.index-1].threshold = aDataSet.data;
						eeprom_update_byte(&eeprom_channel_data[aDataSet.index-1].threshold, channel_data[aDataSet.index-1].threshold);
						break;
						
					case 77:							// backlight and contrast
						//if (aDataSet.index == 1)
						//{
							//alphaDisplay.backlight = aDataSet.data;
							//timer_setPwm(BACKLIGHT,alphaDisplay.backlight);
							//eeprom_update_byte(&eeprom_alphaDisplay.backlight, alphaDisplay.backlight);
						//}
						//else if (aDataSet.index == 2)
						//{
							//alphaDisplay.contrast = aDataSet.data;
							//alpha_setContrast(alphaDisplay.contrast);
							//eeprom_update_byte(&eeprom_alphaDisplay.contrast, alphaDisplay.contrast);
						//}
						break;
					
					case 78:							// led mode
						led_data.mode = aDataSet.data;
						eeprom_update_byte(&eeprom_led_data.mode, led_data.mode);
						ledCtr = 0;
						break;
						
					case 79:							// led manual power
						led_data.manualPower[aDataSet.index-1] = aDataSet.data;
						eeprom_update_byte(&eeprom_led_data.manualPower[aDataSet.index-1], led_data.manualPower[aDataSet.index-1]);
						break;
						
					case 80:							// display content
						//alphaDisplay.content[aDataSet.index-1] = aDataSet.data;
						//eeprom_update_byte(&eeprom_alphaDisplay.content[aDataSet.index-1], alphaDisplay.content[aDataSet.index-1]);
						break;
						
					case 81:							// display screens
						//alphaDisplay.screen[aDataSet.index-1] = aDataSet.data;
						//eeprom_update_byte(&eeprom_alphaDisplay.screen[aDataSet.index-1], alphaDisplay.screen[aDataSet.index-1]);
						break;												
				}				
			}
			
			if (aDataSet.dA == SERIALPORT_ASK)			// new ask
			{
				aDataSet.dA = SERIALPORT_DATA;			// return always data
				switch(aDataSet.id)
				{
					case 0:								// ask for everything
						aDataSet.id = 02;
						aDataSet.index = 1;
						aDataSet.data = FIRMWAREVERSION;
						serialport_writeCarriage(&aDataSet);
						aDataSet.index = 2;
						aDataSet.data = EEPROMVERSION;
						serialport_writeCarriage(&aDataSet);
						for (i=0;i<24;i++)
						{
							aDataSet.id = 11;			// 11
							aDataSet.index = i+1;
							aDataSet.data = channel_data[0].minTemp[i];
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 21;			// 21
							aDataSet.data = channel_data[0].maxTemp[i];
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 12;			// 12
							aDataSet.data = channel_data[1].minTemp[i];
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 22;			// 22
							aDataSet.data = channel_data[1].maxTemp[i];
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 13;			// 13
							aDataSet.data = channel_data[2].minTemp[i];
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 23;			// 23
							aDataSet.data = channel_data[2].maxTemp[i];
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 14;			// 14
							aDataSet.data = channel_data[3].minTemp[i];
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 24;			// 24
							aDataSet.data = channel_data[3].maxTemp[i];
							serialport_writeCarriage(&aDataSet);							
						}
						aDataSet.id = 31;				// 31
						for (i=0;i<8;i++)
						{
							aDataSet.index = i+17;
							aDataSet.data = spare_temperature[i];
							serialport_writeCarriage(&aDataSet);
						}						
						aDataSet.id = 32;				// 32
						aDataSet.index = 1;
						aDataSet.data = tempSensor_getOneWireAmount();
						serialport_writeCarriage(&aDataSet);												
						aDataSet.id = 33;				// 33
						for (i=0;i<(8*tempSensor_getOneWireAmount());i++)
						{
							aDataSet.index = i+1;
							aDataSet.data = tempSensor_getOneWireID(i);
							serialport_writeCarriage(&aDataSet);
						}						
						aDataSet.id = 51;				// 51
						aDataSet.index = 1;
						aDataSet.data = alert_data.overtempEnable;
						serialport_writeCarriage(&aDataSet);
						aDataSet.index = 2;
						aDataSet.data = alert_data.fanblockEnable;
						serialport_writeCarriage(&aDataSet);
						aDataSet.id = 70;				// 70
						aDataSet.index = 1;
						aDataSet.data = pulsePerLiter;
						serialport_writeCarriage(&aDataSet);
						aDataSet.index = 2;
						aDataSet.data = alert_data.minWaterFlow;
						serialport_writeCarriage(&aDataSet);
						for (i=0;i<4;i++)
						{
							aDataSet.id = 71;				// 71
							aDataSet.index = i+1;
							aDataSet.data = channel_data[i].manualPower;
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 72;				// 72
							aDataSet.data = channel_data[i].startupTime;
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 73;				// 73
							aDataSet.data = channel_data[i].minimumPower;
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 74;				// 74
							aDataSet.data = channel_data[i].automaticMode;
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 75;				// 75
							aDataSet.data = channel_data[i].stopEnable;
							serialport_writeCarriage(&aDataSet);
							aDataSet.id = 76;				// 76
							aDataSet.data = channel_data[i].threshold;
							serialport_writeCarriage(&aDataSet);							
						}
						//aDataSet.id = 77;					// 77
						//aDataSet.index = 1;
						//aDataSet.data = alphaDisplay.backlight;
						//serialport_writeCarriage(&aDataSet);
						//aDataSet.index = 2;
						//aDataSet.data = alphaDisplay.contrast;
						//serialport_writeCarriage(&aDataSet);						
						aDataSet.id = 78;					// 78
						aDataSet.index = 1;
						aDataSet.data = led_data.mode;
						serialport_writeCarriage(&aDataSet);
						aDataSet.id = 79;					// 79
						for (i=0;i<3;i++)
						{
							aDataSet.index = i+1;
							aDataSet.data = led_data.manualPower[i];
							serialport_writeCarriage(&aDataSet);
						}
						//aDataSet.id = 80;					// 80
						//for (i=0;i<255;i++)				
						//{
							//aDataSet.index = i+1;
							//aDataSet.data = alphaDisplay.content[i];
							//serialport_writeCarriage(&aDataSet);
						//}
						//aDataSet.id = 81;					// 81
						//for (i=0;i<31;i++)
						//{
							//aDataSet.index = i+1;
							//aDataSet.data = alphaDisplay.screen[i];
							//serialport_writeCarriage(&aDataSet);
						//}									
						break;
					
					case 02:
						if (aDataSet.index == 0)		// ask for all of them
						{
							aDataSet.index = 1;
							aDataSet.data = FIRMWAREVERSION;
							serialport_writeCarriage(&aDataSet);
							aDataSet.index = 2;
							aDataSet.data = EEPROMVERSION;
							serialport_writeCarriage(&aDataSet);
						}
						else if (aDataSet.index == 1)
						{
							aDataSet.data = FIRMWAREVERSION;
							serialport_writeCarriage(&aDataSet);
						}
						else if (aDataSet.index == 2)
						{
							aDataSet.data = EEPROMVERSION;
							serialport_writeCarriage(&aDataSet);
						}
						break;
					
					case 11:
						if (aDataSet.index == 0)		// ask for all of them
						{							
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[0].minTemp[i];
								serialport_writeCarriage(&aDataSet);	
							}							
						} 
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[0].minTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}					
						break;
						
					case 21:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[0].maxTemp[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[0].maxTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
					
					case 12:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[1].minTemp[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[1].minTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 22:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[1].maxTemp[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[1].maxTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
					
					case 13:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[2].minTemp[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[2].minTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 23:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[2].maxTemp[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[2].maxTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 14:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[3].minTemp[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[3].minTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 24:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<24;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[3].maxTemp[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[3].maxTemp[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
					
					case 31:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<8;i++)
							{
								aDataSet.index = i+17;
								aDataSet.data = spare_temperature[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = spare_temperature[aDataSet.index-17];
							serialport_writeCarriage(&aDataSet);
						}
						break;
					
					case 32:
						aDataSet.index = 1;
						aDataSet.data = tempSensor_getOneWireAmount();
						serialport_writeCarriage(&aDataSet);
						break;
					
					case 33:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<(8*tempSensor_getOneWireAmount());i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = tempSensor_getOneWireID(i);
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = tempSensor_getOneWireID(aDataSet.index-1);
							serialport_writeCarriage(&aDataSet);
						}
						break;
					
					case 51:
						switch (aDataSet.index)
						{
							case 0:
								aDataSet.index = 1;
								aDataSet.data = alert_data.overtempEnable;
								serialport_writeCarriage(&aDataSet);
								aDataSet.index = 2;
								aDataSet.data = alert_data.fanblockEnable;
								serialport_writeCarriage(&aDataSet);
								break;
								
							case 1:
								aDataSet.data = alert_data.overtempEnable;
								serialport_writeCarriage(&aDataSet);
								break;
								
							case 2:
								aDataSet.data = alert_data.fanblockEnable;
								serialport_writeCarriage(&aDataSet);
								break;			
						}
						break;
					
					case 70:
						switch (aDataSet.index)
						{
							case 0:
								aDataSet.index = 1;
								aDataSet.data = pulsePerLiter;
								serialport_writeCarriage(&aDataSet);
								aDataSet.index = 2;
								aDataSet.data = alert_data.minWaterFlow;
								serialport_writeCarriage(&aDataSet);
								break;
							
							case 1:
								aDataSet.data = pulsePerLiter;
								serialport_writeCarriage(&aDataSet);
								break;
							
							case 2:
								aDataSet.data = alert_data.minWaterFlow;
								serialport_writeCarriage(&aDataSet);
								break;
						}
						break;
					
					case 71:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<4;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[i].manualPower;
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[aDataSet.index-1].manualPower;
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 72:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<4;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[i].startupTime;
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[aDataSet.index-1].startupTime;
							serialport_writeCarriage(&aDataSet);
						}
						break;
					
					case 73:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<4;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[i].minimumPower;
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[aDataSet.index-1].minimumPower;
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 74:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<4;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[i].automaticMode;
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[aDataSet.index-1].automaticMode;
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 75:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<4;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[i].stopEnable;
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[aDataSet.index-1].stopEnable;
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 76:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<4;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = channel_data[i].threshold;
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = channel_data[aDataSet.index-1].threshold;
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 77:							// display backlight and contrast
						//switch (aDataSet.index)
						//{
							//case 0:
								//aDataSet.index = 1;
								//aDataSet.data = alphaDisplay.backlight;
								//serialport_writeCarriage(&aDataSet);
								//aDataSet.index = 2;
								//aDataSet.data = alphaDisplay.contrast;
								//serialport_writeCarriage(&aDataSet);
								//break;
						//
							//case 1:
								//aDataSet.data = alphaDisplay.backlight;
								//serialport_writeCarriage(&aDataSet);
								//break;
						//
							//case 2:
								//aDataSet.data = alphaDisplay.contrast;
								//serialport_writeCarriage(&aDataSet);
								//break;
						//}
						break;
						
					case 78:
						aDataSet.index = 1;
						aDataSet.data = led_data.mode;
						serialport_writeCarriage(&aDataSet);
						break;
						
					case 79:
						if (aDataSet.index == 0)		// ask for all of them
						{
							for (i=0;i<3;i++)
							{
								aDataSet.index = i+1;
								aDataSet.data = led_data.manualPower[i];
								serialport_writeCarriage(&aDataSet);
							}
						}
						else							// ask for a specific one
						{
							aDataSet.data = led_data.manualPower[aDataSet.index-1];
							serialport_writeCarriage(&aDataSet);
						}
						break;
						
					case 80:						// display content
						//if (aDataSet.index == 0)		// ask for all of them
						//{
							//for (i=0;i<255;i++)
							//{
								//aDataSet.index = i+1;
								//aDataSet.data = alphaDisplay.content[i];
								//serialport_writeCarriage(&aDataSet);
							//}
						//}
						//else							// ask for a specific one
						//{
							//aDataSet.data = alphaDisplay.content[aDataSet.index-1];
							//serialport_writeCarriage(&aDataSet);
						//}
						break;
						
					case 81:						// display screens
						//if (aDataSet.index == 0)		// ask for all of them
						//{
							//for (i=0;i<31;i++)
							//{
								//aDataSet.index = i+1;
								//aDataSet.data = alphaDisplay.screen[i];
								//serialport_writeCarriage(&aDataSet);
							//}
						//}
						//else							// ask for a specific one
						//{
							//aDataSet.data = alphaDisplay.screen[aDataSet.index-1];
							//serialport_writeCarriage(&aDataSet);
						//}
						break;
					
				}				
			}
		
									
		}	
		
    }		// while(1) end
}			// main end

