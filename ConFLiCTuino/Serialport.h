/**
 * @file Serialport.h
 *
 *@brief Serialport communication via RS232.
 *
 * This library contains functions to communicate with the PC via RS232.\n
 * It uses USART0 with 8N1 and 57600 Baud. Sending and transmitting is interrupt driven.
 *
 * @author Robert Steigemann
 * @date 14 Oct 2013 
 */



#ifndef SERIALPORT_H_
#define SERIALPORT_H_
//#include <stdbool.h>

/**
 * @brief A carriage contains one data for communication.
 */
typedef struct{						
	uint8_t dA;									/**< data/ask (DA) identifier */
	uint8_t id;									/**< ID of the data (compare it to a array name) */
	uint8_t index;								/**< number of the data (compare it to an array index) */
	uint16_t data;								/**< the data */
}CARRIAGE;

#define SERIALPORT_DATA 0								// meaning of the DA identifier
#define SERIALPORT_ASK 1

#define MAX_CARRIAGE_LENGTH 16					// maximum length of the carriage string including CR

/**
 * @brief This function initializes the USART C1 to 8N1 with 57600 Baud and enables the RXC interrupt.
 */
void serialport_init(void);

/**
 * @brief This function you send an array (string).
 *
 * It uses 64 byte buffer. It is possible to overrun the buffer, don't send more than 64 bytes at one Time!
 * Check the buffer space with the serialport_checkSpace() function, before you use this function.\n
 * Sending is interrupt driven and don't stop the controller. The function uses only the arrayLength.
 * String end (0x00) would not be recognized!
 *
 * @param *anArray The pointer to the array with the data to send.
 * @param arrayLength The number of bytes to send.
 */
void serialport_write(uint8_t *anArray, uint8_t arrayLength);


/**
 * @brief This function check how many bytes are free in the transmit buffer.
 *
 * @return Number of free bytes in the transmit buffer.
 */ 
uint8_t serialport_checkSpace(void);

/**
 * @brief This function gives you all received data to the next CR (carriage return) in string format.
 *
 * To get the string, it is necessary that your array is big enough to hold all data to the CR.
 * If is not (maxLength reached, before CR is) this function return 0 and data is lost.
 * The maxLength means all chars in the string including 0x00, so it should equals the array length!
 *
 * @param *anArray The pointer to the array assigned with the data. It have to be big enough to store all data received!
 * @param maxLength The length of the array. Correct value is necessary to prevent array overflow.
 *
 * @return The amount of characters stored in the array including the string end sign 0x00!
 *		   If 0 is returned, no CR is received or the array is to small.
 */
uint8_t serialport_getLastString(uint8_t *anArray, uint8_t maxLength);

/**
 * @brief This function gives you the last received carriage.
 *
 * This function read out the last received string (witch should have the carriage format) and convert it to a carriage.
 * It is protected against wrong string codes. The function use the serialport_getLastString() function.
 *
 * @param aCARRIAGE The pointer to the carriage witch is assigned with the new data.
 *
 * @return - 0    everything fine, carriage received
 *		   - 1    no CR is received until the last execution (no new data available)
 *		   - >1   something is received, but isn't valid (details in implementation)
 */
uint8_t serialport_getLastCarriage(CARRIAGE* aCARRIAGE);

/**
 * @brief This function send a carriage as string in the carriage format.
 *
 * It transform a carriage into a valid string and send it vie the serialport_write() function.
 * It use serialport_checkSpace() function too and wait for enough space if the buffer is full.
 * So in every case all data are transmitted and buffer overflow is not possible.
 * Be careful when you send many carriages successively because the function WAIT if the buffer is full and no other
 * code is executed until all carriages are copied into transmit buffer! 
 *
 * @param aCARRIAGE The pointer to the carriage witch is transmitted.
 */   
void serialport_writeCarriage(CARRIAGE* aCARRIAGE);

/**
 * @brief This function is like printf.
 *
 * It uses 64 byte buffer. Your output have to be shorter than 64 chars.
 * It use serialport_checkSpace() function too and wait for enough space if the buffer is full.\n
 * Sending is interrupt driven and don't stop the controller.
 *
 * @param *anArray The pointer to the array with the data to send.
 * @param arrayLength The number of bytes to send.
 */
void serialport_printf(const char *__fmt, ...);


#endif /* SERIALPORT_H_ */