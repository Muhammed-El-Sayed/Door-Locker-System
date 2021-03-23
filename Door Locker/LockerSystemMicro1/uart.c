 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.c
 *
 * Description: Source file for the UART AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#include "uart.h"


/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
void UART_init(const UART_ConfigType * Config_Ptr)
{
	/* U2X for double transmission speed */

	UCSRA = (UCSRA & 0xFD) | ((Config_Ptr->ActivateDoubleAsyncSpeed)<<1);

	/************************** UCSRB Description **************************
	 * RXCIE = 0 Disable USART RX Complete Interrupt Enable
	 * TXCIE = 0 Disable USART Tx Complete Interrupt Enable
	 * UDRIE = 0 Disable USART Data Register Empty Interrupt Enable
	 * RXEN  = 1 Receiver Enable
	 * TXEN  = 1 Transmitter Enable
	 * UCSZ2 for data mode
	 * RXB8 & TXB8 not used for 8-bit data mode
	 ***********************************************************************/
	UCSRB = (1<<RXEN) | (1<<TXEN);
	UCSRB = ((UCSRB&0XFB) | ( ((Config_Ptr->CharacterSize)& 0b100) ));

	/************************** UCSRC Description **************************
	 * URSEL   = 1 The URSEL must be one when writing the UCSRC
	 * UMSEL    for Asynchronous or Synchronous Operation
	 * UPM1:0   parity bit mode
	 * USBS     stop bit no.
	 * UCSZ1:0  for data mode
	 * UCPOL   = 0 Used with the Synchronous operation only
	 ***********************************************************************/
	UCSRC = (1<<URSEL);
	UCSRC = ((UCSRC&0XBF) | ((Config_Ptr->OperatingMode)<<6));
	UCSRC = ((UCSRC&0XCF) | ((Config_Ptr->ParityBitMode)<<4));
	UCSRC = ((UCSRC&0XF7) | ((Config_Ptr->StopBitNo)<<3));
	UCSRC = ((UCSRC&0XF9) | ( ((Config_Ptr->CharacterSize)& 0b011)<<1));



 if((Config_Ptr->OperatingMode) == ASYNCHRONOUS && (Config_Ptr->ActivateDoubleAsyncSpeed) == YES)
	{
	 uint16 BAUD_PRESCALE = (((F_CPU / ((Config_Ptr->UsartBaudrate) * 8UL))) - 1);
	/* First 8 bits from the BAUD_PRESCALE inside UBRRL and last 4 bits in UBRRH*/
	 UBRRH = BAUD_PRESCALE>>8;
	 UBRRL = BAUD_PRESCALE;
	}

 else if((Config_Ptr->OperatingMode) == ASYNCHRONOUS && (Config_Ptr->ActivateDoubleAsyncSpeed) == NO)
	{
	 uint16 BAUD_PRESCALE = (((F_CPU / ((Config_Ptr->UsartBaudrate) * 16UL))) - 1);
	/* First 8 bits from the BAUD_PRESCALE inside UBRRL and last 4 bits in UBRRH*/
	 UBRRH = BAUD_PRESCALE>>8;
	 UBRRL = BAUD_PRESCALE;
	}

 else if((Config_Ptr->OperatingMode) == SYNCHRONOUS )
	{
	 uint16 BAUD_PRESCALE = (((F_CPU / ((Config_Ptr->UsartBaudrate) * 2UL))) - 1);
	/* First 8 bits from the BAUD_PRESCALE inside UBRRL and last 4 bits in UBRRH*/
	 UBRRH = BAUD_PRESCALE>>8;
	 UBRRL = BAUD_PRESCALE;
	}
}

void UART_sendByte(const uint8 data)
{
	/* UDRE flag is set when the Tx buffer (UDR) is empty and ready for
	 * transmitting a new byte so wait until this flag is set to one */
	while(BIT_IS_CLEAR(UCSRA,UDRE)){}
	/* Put the required data in the UDR register and it also clear the UDRE flag as
	 * the UDR register is not empty now */
	UDR = data;
	/************************* Another Method *************************
	UDR = data;
	while(BIT_IS_CLEAR(UCSRA,TXC)){} // Wait until the transimission is complete TXC = 1
	SET_BIT(UCSRA,TXC); // Clear the TXC flag
	*******************************************************************/
}

uint8 UART_recieveByte(void)
{
	/* RXC flag is set when the UART receive data so wait until this
	 * flag is set to one */
	while(BIT_IS_CLEAR(UCSRA,RXC)){}
	/* Read the received data from the Rx buffer (UDR) and the RXC flag
	   will be cleared after read this data */
    return UDR;
}

void UART_sendString(const uint8 *Str)
{
	uint8 i = 0;
	while(Str[i] != '\0')
	{
		UART_sendByte(Str[i]);
		i++;
	}
	/************************* Another Method *************************
	while(*Str != '\0')
	{
		UART_sendByte(*Str);
		Str++;
	}
	*******************************************************************/
}

void UART_receiveString(uint8 *Str)
{
	uint8 i = 0;
	Str[i] = UART_recieveByte();
	while(Str[i] != '#')
	{
		i++;
		Str[i] = UART_recieveByte();
	}
	Str[i] = '\0';
}
