/**
 * @file Serialport.c
 *
 *@brief Serialport communication via RS232.
 *
 * This library contains functions to communicate with the PC via RS232.\n
 *
 * @author Robert Steigemann
 * @date 03.05.2014
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "Serialport.h"


static volatile uint8_t txd_buffer[64];								/**< the 64 Byte transmit buffer */
static volatile uint8_t txd_point_tail = 0;						    /**< pointer offset to the next byte to transmit in buffer */
static volatile uint8_t txd_point_head = 0;							/**< pointer offset to the next position to write in transmit buffer */

static volatile uint8_t rxd_buffer[256];							/**< the 256 Byte receive buffer */
static volatile uint8_t rxd_point_tail = 0;						    /**< pointer offset to the last read byte in buffer */
static volatile uint8_t rxd_point_head = 0;							/**< pointer offset to the next position to write in receive buffer */


// This function add an array to the transmit buffer and control the DRE interrupt.
void serialport_write(uint8_t *anArray, uint8_t arrayLength)
{	
	uint8_t counter;												// the count variable over the array length
	
	for (counter=0;counter<arrayLength;counter++)					// for each byte in the array
	{				
		UCSR0B &= 0b11011111;										// deactivate the DRE (data register empty) interrupt 			
		*(txd_buffer+txd_point_head) = *(anArray+counter);			// add a byte from the array to the transmit buffer		
		txd_point_head = 0b00111111 & (txd_point_head + 1);			// increment the last byte "pointer" and mask it for 64 to match the buffer size
		UCSR0B |= 0b00100000;										// activate the DRE interrupt 		
	}			
}


// This interrupt is executed, when the data register is empty. It put the next byte into the data register of the UART to transmit it.
// This interrupt is only enabled, if the transmit buffer hold at least one byte to transmit!
ISR(USART0_UDRE_vect)
{		
	UDR0 = txd_buffer[txd_point_tail];								// put the next byte into the data register	
	txd_point_tail = 0b00111111 & (txd_point_tail + 1);				// increment the next byte pointer and mask it for 64 to match the buffer size
	
	// if the last byte is transmitting	
	if (txd_point_tail==txd_point_head){
		UCSR0B &= 0b11011111;										// deactivate the DRE interrupt 		
	}		
}


// This function check how many space are in the transmit buffer.
uint8_t serialport_checkSpace(void)
{
	if ((UCSR0B & 0b00100000) == 0)									// there is no data to transmit, because the interrupt is off
	{
		return 64;													// so simply return 64 bytes are free
	} else if (txd_point_head == txd_point_tail)					// buffer is complete full!
	{
		return 0;
	} else															// calculate free buffer
	{
		return 64 - ((txd_point_head - txd_point_tail) & 0b00111111);			
	}
}


// This interrupt is executed, when a byte is received. It put it on the head of the receive buffer and increment the head pointer.
ISR(USART0_RX_vect)
{	
	rxd_buffer[rxd_point_head] = UDR0;								// copy byte from data register to receive buffer			
	rxd_point_head++;												// increment the receive head pointer	
}


// This function gives you all received data to the next CR (carriage return) in string format.
// To get the string, it is necessary that your array is big enough to hold all data to the CR.
// If is not (maxLength reached, before CR is) this function return 0 and data is lost.
uint8_t serialport_getLastString(uint8_t *anArray, uint8_t maxLength)
{	
	uint8_t counter;												// the count variable over the array length
	uint8_t cr_is_there = 0;										// becomes true if we found a CR
	
	// run through all filled buffer bytes and search for a CR
	for (counter=rxd_point_tail;counter!=rxd_point_head;counter++)
	{						
		if (rxd_buffer[counter]==13) {								// 13 == CR (carriage return)
			cr_is_there = 1;										// CR found!
			break;													// don't search for a second one
		}		
	}
	
	// if we have a CR
	if (cr_is_there)
	{		
		counter = 0;												// clear counter		
				
		while (rxd_buffer[rxd_point_tail]!=13)						// until we reach the CR
		{			
			*(anArray+counter) = rxd_buffer[rxd_point_tail];		// copy data to array			
			rxd_point_tail++;										// increment tail
			counter++;												// and the counter
			
			// If the array is full and CR is not reached!
			if ((counter == (maxLength-1)) && (rxd_buffer[rxd_point_tail]!=13))
			{
				*(anArray+counter) = 0;								// add the '\0' to the end of string
				rxd_point_tail++;									// also need to increment the tail				
				return 0;											// return 0 (change this if you want)
			}									
		}
				
		*(anArray+counter) = 0;										// add the '\0' to the end of string	
		rxd_point_tail++;											// need to increment the tail ones more
		
		return counter+1;											// return the received characters + '\0'
				
	} else															// we haven't found a CR
	{																
		return 0;													// nothing received to return		
	}	
}


// This function initializes the USART C1 to 8N1 with 57600 Baud and enables the RXC interrupt.
void serialport_init(void)
{
	// According to the ATmega2560 data sheet page 230 use U2X = 0 and UBRR = 16
	// for 57600 Baud with 16 MHz oscillator. Error is usually not acceptable with 2.1%.
	// But the ATmega16U2 on the Arduino 2560 have the same error, so it works fine!
	
	UBRR0 = 16;														// setup baud rate generator
	UCSR0A = 0b00000000;											// reset U2X0
	UCSR0C = 0b00000110;											// 8N1
	UCSR0B = 0b10011000;											// enable receive interrupt, receiver and transmitter
}


// This function read out the last received string (witch should have the carriage format) and convert it to a carriage.
// It is protected against wrong string codes. See error code table in the header. It uses the serialport_getLastString(..) function.
uint8_t serialport_getLastCarriage(CARRIAGE* aCARRIAGE)
{	
	uint8_t recievedString[MAX_CARRIAGE_LENGTH];					// string extracted out of the received data
	uint8_t Substring[5];											// extracted substring (hold values)
	uint8_t stringLength;									
	uint8_t counter = 0;
	uint8_t firstData = 7;											// position of the first data character 
	uint8_t countHashtag = 0;										// count the '#' 
	uint8_t thirdHashtagPosition = 0;								// position of the third '#'
	
	// use the function for receiving a string	
	stringLength = serialport_getLastString(recievedString, MAX_CARRIAGE_LENGTH);
		
	if(stringLength==0) return 1;									// error code "1" - no new carriage
	if(stringLength<10) return 2;									// error code "2" - string is to short
	if(stringLength>MAX_CARRIAGE_LENGTH) return 3;					// error code "3" - string is to long!!!
																	// this should not be possible!
	// check the format of the received string
	countHashtag = 0;		
	for(counter=0;counter<stringLength;counter++){					// for each character
		if(recievedString[counter]=='#') {							// character is a '#'
			countHashtag++;
			if(countHashtag==1 && counter!=1) return 4;				// first # on wrong position
			if(countHashtag==2 && counter!=4) return 5;				// second # on wrong position
			if(countHashtag==3 && counter<6) return 6;				// third # to early
			if(countHashtag==3 && counter>8) return 7;				// third # to late
			if(countHashtag==3) thirdHashtagPosition = counter;		
			if(countHashtag==5 && (counter-thirdHashtagPosition)<2) return 8;	// fourth # to early
			if(countHashtag==5 && (counter-thirdHashtagPosition)>6) return 9;	// fourth # to late						
		}					
	}
	if(countHashtag!=4) return 10;									// error code "4" - format does not match
	
	
	// if you reach this, the string seems to be valid!
			
	aCARRIAGE->dA = recievedString[0] - '0';						// store the DA identifier							
	aCARRIAGE->id = (10 * (recievedString[2] - '0') + recievedString[3] - '0');	// store the ID
	
	// extract the number and count its length	
	for(counter=0;counter<3;counter++){			
		if (recievedString[5+counter]=='#') break;
		Substring[counter] = recievedString[5+counter];			
	}
	
	// convert the number in the string to a uint_8				
	switch (counter) {
			
		case 3:														// three digits
			aCARRIAGE->index = (100 * (Substring[0] - '0'));
			aCARRIAGE->index += (10 * (Substring[1] - '0'));
			aCARRIAGE->index += (Substring[2] - '0');
			firstData += 2;
			break;
			
		case 2:														// two digits
			aCARRIAGE->index = (10 * (Substring[0] - '0'));
			aCARRIAGE->index += Substring[1] - '0';
			firstData++;
			break;
				
		case 1:														// one digit
			aCARRIAGE->index = Substring[0] - '0';
			break;
				
		default:													// no digits?
			return 11;												// shouldn't possible
	}
		
	// extract the data and count its length
	for(counter=0;counter<5;counter++){
		if (recievedString[firstData+counter]=='#') break;
		Substring[counter] = recievedString[firstData+counter];
	}
	
	// convert the data in the string to a uint_16
	switch (counter) {
		
		case 5:														// five digits
			aCARRIAGE->data = (10000 * (Substring[0] - '0'));
			aCARRIAGE->data += (1000 * (Substring[1] - '0'));
			aCARRIAGE->data += (100 * (Substring[2] - '0'));
			aCARRIAGE->data += (10 * (Substring[3] - '0'));
			aCARRIAGE->data += (Substring[4] - '0');
			break;
		
		case 4:														// four digits
			aCARRIAGE->data = (1000 * (Substring[0] - '0'));
			aCARRIAGE->data += (100 * (Substring[1] - '0'));
			aCARRIAGE->data += (10 * (Substring[2] - '0'));
			aCARRIAGE->data += (Substring[3] - '0');
			break;
						
		case 3:														// three digits
			aCARRIAGE->data = (100 * (Substring[0] - '0'));
			aCARRIAGE->data += (10 * (Substring[1] - '0'));
			aCARRIAGE->data += (Substring[2] - '0');
			break;
				
		case 2:														// two digits
			aCARRIAGE->data = (10 * (Substring[0] - '0'));
			aCARRIAGE->data += Substring[1] - '0';
			break;
				
		case 1:														// one digit
			aCARRIAGE->data = Substring[0] - '0';
			break;
		
		default:													// no digits?
			return 12;
	}	
		
	return 0;														// everything fine, new carriage received		
}


// This function send a carriage as string in the carriage format.
void serialport_writeCarriage(CARRIAGE* aCARRIAGE)
{
	uint8_t transmitString[16];
	uint8_t stringLength = 0;
	uint8_t numberDigit = 0;
	uint8_t dataDigit = 0;
	
	transmitString[0] = aCARRIAGE->dA + '0';
	transmitString[1] = '#';
	transmitString[2] = (aCARRIAGE->id / 10) + '0';
	transmitString[3] = (aCARRIAGE->id % 10) + '0';
	transmitString[4] = '#';
	
	if (aCARRIAGE->index < 10)										// one digit
	{
		numberDigit = 1;
		transmitString[5] = aCARRIAGE->index + '0';
						
	} else if (aCARRIAGE->index < 100)								// two digits
	{
		numberDigit = 2;
		transmitString[5] = (aCARRIAGE->index / 10) + '0';
		transmitString[6] = (aCARRIAGE->index % 10) + '0';	
					
	} else															// three digits
	{
		numberDigit = 3;
		transmitString[5] = (aCARRIAGE->index / 100) + '0';
		transmitString[6] = ((aCARRIAGE->index % 100) / 10) + '0';
		transmitString[7] = (aCARRIAGE->index % 10) + '0';
	}
	
	transmitString[numberDigit+5] = '#';
	
	if (aCARRIAGE->data < 10)										// one digit
	{
		dataDigit = 1;
		transmitString[numberDigit+6] = aCARRIAGE->data + '0';
		
	} else if (aCARRIAGE->data < 100)								// two digits
	{
		dataDigit = 2;
		transmitString[numberDigit+6] = (aCARRIAGE->data / 10) + '0';
		transmitString[numberDigit+7] = (aCARRIAGE->data % 10) + '0';
		
	} else if (aCARRIAGE->data < 1000)								// three digits
	{
		dataDigit = 3;
		transmitString[numberDigit+6] = (aCARRIAGE->data / 100) + '0';
		transmitString[numberDigit+7] = ((aCARRIAGE->data % 100) / 10) + '0';
		transmitString[numberDigit+8] = (aCARRIAGE->data % 10) + '0';
		
	} else if (aCARRIAGE->data < 10000)								// four digits
	{
		dataDigit = 4;
		transmitString[numberDigit+6] = (aCARRIAGE->data / 1000) + '0';
		transmitString[numberDigit+7] = ((aCARRIAGE->data % 1000) / 100) + '0';
		transmitString[numberDigit+8] = ((aCARRIAGE->data % 100) / 10) + '0';
		transmitString[numberDigit+9] = (aCARRIAGE->data % 10) + '0';
		
	} else															// five digits
	{
		dataDigit = 5;
		transmitString[numberDigit+6] = (aCARRIAGE->data / 10000) + '0';
		transmitString[numberDigit+7] = ((aCARRIAGE->data % 10000) / 1000) + '0';
		transmitString[numberDigit+8] = ((aCARRIAGE->data % 1000) / 100) + '0';
		transmitString[numberDigit+9] = ((aCARRIAGE->data % 100) / 10) + '0';
		transmitString[numberDigit+10] = (aCARRIAGE->data % 10) + '0';
				
	}
	
	transmitString[numberDigit+dataDigit+6] = '#';
	transmitString[numberDigit+dataDigit+7] = 13;					// CR
	
	stringLength = numberDigit + dataDigit + 8;
	
	while (stringLength>serialport_checkSpace());					// wait until transmit buffer is free enough
	
	serialport_write(transmitString, stringLength);					// send the string		
}

// printf for uart
void serialport_printf(const char* __fmt, ...)
{
	char aString[64];
	va_list argumentlist;
	va_start(argumentlist, __fmt);
	sprintf(aString,__fmt, argumentlist);
	va_end(argumentlist);	
	while(strlen(aString)>serialport_checkSpace());
	serialport_write((uint8_t *)aString, strlen(aString));	
}