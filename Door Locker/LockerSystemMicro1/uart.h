 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"


/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/

typedef enum
{
	NO,YES
}Double_Transmission_Speed;

typedef enum
{
	ASYNCHRONOUS,SYNCHRONOUS
}Operating_Mode;

typedef enum
{
	FIVE,SIX,SEVEN,EIGHT,NINE=7
}Character_Size;

typedef enum
{
	DISABLED,EVEN=2,ODD
}Parity_Bit_Mode;

typedef enum
{
	ONE,TWO
}Stop_Bit_No;


typedef struct
{
	Operating_Mode OperatingMode;
	Double_Transmission_Speed ActivateDoubleAsyncSpeed;
	Character_Size CharacterSize;
	Parity_Bit_Mode ParityBitMode;
	Stop_Bit_No StopBitNo;
	uint16 UsartBaudrate;

}UART_ConfigType;


/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void UART_init(const UART_ConfigType * Config_Ptr);

void UART_sendByte(const uint8 data);

uint8 UART_recieveByte(void);

void UART_sendString(const uint8 *Str);

void UART_receiveString(uint8 *Str); // Receive until #

#endif /* UART_H_ */
