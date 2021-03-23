
/******************************************************************************
 *
 * Module: Microcontroller2
 *
 * File Name: micro2.c
 *
 * Description: Source file for the Micro2
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/


#include"uart.h"
#include"external_eeprom.h"
#include"timer.h"

#define MICRO1_IS_READY 0X01
#define MICRO2_IS_READY 0X10
#define PASSWORD_CORRECT 0X11
#define PASSWORD_WRONG 0X00
#define NEWPASSFLAG '!'
#define CHECKPASSFLAG '`'
#define BUZZER ']'
#define DOOR '['


uint8 flag=0;

void BuzzerAndTimer1_Off(void)
{

	/*Turn off buzzer and timer*/
	CLEAR_BIT(PORTB,PB2);
	TIMER_ConfigType timerConfig ={TIMER1,A,NORMAL,0,NO_CLOCK,0}; //Timer1overflowTime=8.4 sec
	Timer_init(&timerConfig);

}

void MotorRotationAndTimer1_off(void)
{


	/*If motor was rotating clockwise*/
	if( (BIT_IS_SET(PINA,PA1)) && (BIT_IS_CLEAR(PINA,PA0)) )
	{
		/*stops it*/
		CLEAR_BIT(PORTA,PA0);
		CLEAR_BIT(PORTA,PA1);
	}
	/*if motor was rotating anticlockwise*/
	else if ( (BIT_IS_CLEAR(PINA,PA1)) && (BIT_IS_SET(PINA,PA0)) )
	{
		/*stops motor and turn off timer and make flag =1 so the code inside if statement that
		 * (flag == 0) will not be executed*/
		CLEAR_BIT(PORTA,PA0);
		CLEAR_BIT(PORTA,PA1);
		TIMER_ConfigType timerConfig ={TIMER1,A,COMPARE,0,NO_CLOCK,0,DISCONNECT};
		Timer_init(&timerConfig);
		flag=1;

	}
	/*This will be verified if motor was rotating clockwise then after 15 sec code jumps to this
	 * function and the first if statement in this function will be executed so motor stops
	 * and flag value all of this = 0  as it doesnot changed as motor still not reach to state
	 * that it rotates anticlockwise (second if statement in this function)*/
	if(flag==0){
		/*turn off timer*/
		TIMER_ConfigType timerConfig1 ={TIMER1,A,COMPARE,0,NO_CLOCK,0,DISCONNECT};
		Timer_init(&timerConfig1);

		/*delay 3 sec so motor will stop for 3 sec*/
		_delay_ms(3000);

		/*rotate motor anticlockwise*/
		CLEAR_BIT(PORTA,PA1);
		SET_BIT(PORTA,PA0);
		/*Turn timer on again so after 15 sec code will jump to this function but motor state
		 * is rotating anticlockwise so second if statement in this function will be executed*/
		TIMER_ConfigType timerConfig2 ={TIMER1,A,COMPARE,0,F_CPU_1024,65534,DISCONNECT};
		Timer_init(&timerConfig2);
	}


}

int main(void)
{
	/*Enable Globle Interrupt Enable*/
	sei();
	/*Initialize Uart as
	 * Mode: Asynchronous
	 * Activate double Asynchronous Speed
	 * Character Size is Eight
	 * Parity bit mode is disabled
	 * Stop bit no is one
	 * baud rate is 9600
	 */
	UART_ConfigType uartConfig = {ASYNCHRONOUS,YES,EIGHT,DISABLED,ONE,9600};
	UART_init(&uartConfig);




	EEPROM_init();

	uint8 key_num;
	uint8 val = 0;


	uint16 eepromAddress=0; /*Initial address to start from it putting password characters where
	each address carry one character*/
	uint8 Password[10];/*Password of system is the first 5 charcaters , password re-entered or
	to be checked with password of the system is in the second 5 characters*/



	Timer1_COMPA_ISR_setCallBack(MotorRotationAndTimer1_off);
	Timer1_Overflow_ISR_setCallBack(BuzzerAndTimer1_Off);

	while(1)
	{
		/*micro1 send NewPassFlag to micro2 so code that executed
			in micro2 is that receives and saves the 1st 5 characters Password sent in first five locations in eeprom
			and the 2nd 5 characters Password saved in second five locations in eeprom*/
		if(UART_recieveByte()== NEWPASSFLAG){
			eepromAddress =0;
			for(uint16 i=1;i<=10;++i){

				key_num = UART_recieveByte();

				EEPROM_writeByte(eepromAddress, key_num);
				_delay_ms(10);
				EEPROM_readByte(eepromAddress, &val);
				Password[eepromAddress] = val;

				++eepromAddress;

			}
		}
		/*micro1 sends checkPassflag so the code in micro2 that will be executed is
		 * the one which makes the 5 characters password sent be received and saved in second
		 * five eeprom locations so it can be compared with old password which
		 * is already saved in first 5 eeprom locations */
		else if(UART_recieveByte()== CHECKPASSFLAG)
		{

			eepromAddress = 5;
			for(uint16 i=1;i<=5;++i){

				key_num = UART_recieveByte();

				EEPROM_writeByte(eepromAddress, key_num);  //Write 0x0F in the external EEPROM
				_delay_ms(10);
				EEPROM_readByte(eepromAddress, &val);  //Read 0x0F from the external EEPROM
				Password[eepromAddress] = val;   //out the read value from the external EEPROM

				++eepromAddress;

			}
		}
		/*micro1 send buzzer flag to micro2 so code of micro 2 will turn buzzer on*/
		else if(UART_recieveByte()== BUZZER)
		{

			/*Make micro pin connected to buzzer is output
			 * and put high so buzzer turns on */
			SET_BIT(DDRB,PB2);
			SET_BIT(PORTB,PB2);

			/*
			 *Initialize Timer
			 *Select Timer 1
			 *Channel A (Will not be used here as we will use normal mode not compare mode)
			 *Normal Mode
			 *initial value of Timer = 0
			 *Freq. of the timer = F_CPU/1024,where F_CPU = 8MHz
			 *output compare value = 0(Will not be used here as we will use normal mode not compare mode)
			 *OC1A is disconnected
			 *Timer overflow after 8.4 sec (ISR of overflow will be executed after 8.4 sec)
			 * */
			TIMER_ConfigType timerConfig ={TIMER1,A,NORMAL,0,F_CPU_1024,0,DISCONNECT};
			Timer_init(&timerConfig);

			/*Timer Here is initialized so after 7*8.4=60 sec , BuzzerAndTimer1_Off fnc
			 * will be executed which will turn off the buzzer and the timer*/

		}
		/*micro1 sent flag DOOR so micro2 open the door where motor rotates clockwise for 15 sec
		 * then stops for 3 sec so person can enter from the door then rotates
		 * anticlockwise to close door for 15 sec then holds motor again*/
		else if(UART_recieveByte()== DOOR)
		{

			/*Make two micro pins connected to two pins of control of motor driver IN1&In2 outputs */
			SET_BIT(DDRA,PA0);
			SET_BIT(DDRA,PA1);
			/*Give order to rotate motor clockwise*/
			CLEAR_BIT(PORTA,PA0);
			SET_BIT(PORTA,PA1);


			flag=0;
			/*
			 *Initialize Timer
			 *Select Timer 1
			 *Channel A
			 *Compare Mode
			 *initial value of Timer = 0
			 *Freq. of the timer = F_CPU/1024,where F_CPU = 8MHz
			 *output compare value approximately = 0XFFFF
			 *OC1A is disconnected
			 *Timer comapare match after 8.4 sec (ISR of compare match of channel A will be executed after 8.4 sec)
			 * */
			TIMER_ConfigType timerConfig ={TIMER1,A,COMPARE,0,F_CPU_1024,65534,DISCONNECT};
			Timer_init(&timerConfig);

			/*Timer Here is initialized so after 2*8.4= 15 sec , MotorRotationAndTimer1_off fnc
			 * will be executed which will holds motor for 3 sec then rotates motor for 15 sec
			 * anticlockwise then holds motor again*/

		}


		/*if Password of system that is saved in first 5 locations of eeprom and first 5 charcters in password
		 * matches with user renter password or password user entered to open door or change password
		 * micro2 send to micro1 that password is correct*/
		if((Password[0]==Password[5]) &&(Password[1]==Password[6]) && (Password[2]==Password[7]) && (Password[3]==Password[8]) && (Password[4]==Password[9]))
		{
			UART_sendByte(PASSWORD_CORRECT);

		}
		/*If 2 passwords not match micro2 send micro1 password wrong flag */
		else
		{
			UART_sendByte(PASSWORD_WRONG);
		}


	}
}
