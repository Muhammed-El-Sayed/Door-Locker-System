/*

 *****************************************************************************
 *
 * Module: Microcontroller1
 *
 * File Name: micro1.c
 *
 * Description: Source file for the Micro1
 *
 * Author: Mohamed Tarek
 *
 ******************************************************************************
 */

#include"lcd.h"
#include"keypad.h"
#include"uart.h"

#define MICRO1_IS_READY 0X01
#define MICRO2_IS_READY 0X10
#define PASSWORD_CORRECT 0X11
#define PASSWORD_WRONG 0X00
#define NEWPASSFLAG '!'
#define CHECKPASSFLAG '`'
#define BUZZER ']'
#define DOOR '['


uint8 key_num; /*key_num is the key value pressed on keypad*/

void sendFiveCharacters(void)
{
	for(int i=1;i<=5;++i){

		key_num = KeyPad_getPressedKey();  /*Get the value of key pressed & save it in Key_num*/
		if((key_num >= 0) && (key_num <= 9)) /*If the value is from 0 to 9*/
		{
			UART_sendByte(key_num); /*Send it to micro2 to be save in eeprom*/
			LCD_displayCharacter('*');/*display it as * on LCD*/
		}
		else /*If user pressed key other than from 0 to 9*/
		{
			/*decrement the iteration variable to be sure that this for-loop loops
			 * in a way so 5 characters from 0 to 9 can be sent to micro2*/
			--i;
		}
		_delay_ms(500); /*new button every 500ms*/
	}
}


int main(void)
{

	LCD_init(); /*Initialize Lcd*/

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

	uint8 controlKey;/* # to open door and * to change Password*/



	LCD_displayString("Enter New Pass:");
	LCD_goToRowColumn(1,0);
	UART_sendByte(NEWPASSFLAG); /*micro1 send NewPassFlag to micro2 so code that executed
	in micro2 is that saves the 1st 5 characters Password sent in first five locations in eeprom
	and the 2nd 5 characters Password sent in second five locations in eeprom*/
	sendFiveCharacters(); /*These are saved in first five locations in eeprom ,Password of the
	system*/


	LCD_clearScreen();
	LCD_displayString("Re-enter Pass:");
	LCD_goToRowColumn(1,0);

	sendFiveCharacters();/*These are saved in second five locations in eeprom,Re-entered Password
	to be sure that it matches with password of the system*/

	uint8 counter=0,flag=1;

	while(1)
	{

		/*If Password in first 5 locations in eeprom is the same as those in second five locations
		 * which means that when User Reenter the Password or when The system asks the user to
		 * enter old password so he can open door or change password The password matches
		 * This If will also be executed when the system asks user to enter Password so he can
		 * open the door or change password and user enters password wrong for 3 times
		 */
		if( (flag==0)||(UART_recieveByte()== PASSWORD_CORRECT) )
		{
			LCD_clearScreen();
			LCD_displayString("*:Change Pass");
			LCD_goToRowColumn(1,0);
			LCD_displayString("#:Open Door");
			_delay_ms(500); /*new button every 500ms*/
			controlKey= KeyPad_getPressedKey();
			if(controlKey == '*') /*change password*/
			{

				counter =0,flag=0;

				do
				{
					/*micro1 sends checkPassflag so the code in micro2 that will be executed is
					 * the one which makes the 5 characters password sent be saved in second
					 * five eeprom locations so it can be compared with old password which
					 * is already saved in first 5 eeprom locations and we make that if user
					 * registered already on the system with password and now he wants to make
					 * action as change password or open door so he must enter old password
					 * so we took the entered password in second 5 eeprom locations to compare
					 * with the real old registered password in first five eeprom locations
					 * We send 2 bytes not one because in micro2 code the code will check the
					 * 1st if statement in which it will receive the first byte but it will not verified
					 * as the byte sent is check pass not new pass flag so the code will enter the
					 * second if but it will waits for byte to receive so we sent second byte
					 */
					UART_sendByte(CHECKPASSFLAG);
					UART_sendByte(CHECKPASSFLAG);
					LCD_clearScreen();
					LCD_displayString("Enter Old Pass");
					LCD_goToRowColumn(1,0);

					sendFiveCharacters();/*User enter password to change password which must matches
					the password of the system registered before*/

					if(UART_recieveByte()== PASSWORD_CORRECT)
					{
						flag=1; /*if password entered by user to change password matches*/
						break;
					}

					++counter;
				}while(counter<3); /*User trials are 3 to enter password to change password*/


				if(flag==0)/*if password entered by user to change password doesn't match for 3 trials */
				{
					/*micro1 send buzzer flag to micro2 so code of micro 2 will turn buzzer on
					 * for 1 minute and we sent 3 bytes as its if statement in micro2 is the
					 * 3rd one so first two if statements in micro2 willnot be executed and
					 * since in each if there is receive byte so we sent 3 bytes*/
					UART_sendByte(BUZZER);
					UART_sendByte(BUZZER);
					UART_sendByte(BUZZER);
				}

				else/*User trials are 3 to enter password to change password*/
				{
					LCD_clearScreen();
					LCD_displayString("Enter New Pass:");
					LCD_goToRowColumn(1,0);

					UART_sendByte(NEWPASSFLAG);
					sendFiveCharacters();


					LCD_clearScreen();
					LCD_displayString("Re-enter Pass:");
					LCD_goToRowColumn(1,0);
					sendFiveCharacters();
				}

			}
			else if(controlKey == '#')/*open door*/
			{

				counter =0,flag=0;

				do
				{

					UART_sendByte(CHECKPASSFLAG);
					UART_sendByte(CHECKPASSFLAG);
					LCD_clearScreen();
					LCD_displayString("Enter Old Pass");
					LCD_goToRowColumn(1,0);

					sendFiveCharacters();

					if(UART_recieveByte()== PASSWORD_CORRECT)
					{
						flag=1;
						break;
					}

					++counter;
				}while(counter<3);


				if(flag==0)
				{

					UART_sendByte(BUZZER);
					UART_sendByte(BUZZER);
					UART_sendByte(BUZZER);
				}

				else
				{
					UART_sendByte(DOOR);
					UART_sendByte(DOOR);
					UART_sendByte(DOOR);
					UART_sendByte(DOOR);
				}

			}

		}
		else
		{ /*At the beginning if user enters password for system then system asks
		    user to reenter it if user didnot re-enter same password the system will
		    continue asking him to re-enter pass after sending message wrong pass till user
		    enters it correct*/
			LCD_clearScreen();
			LCD_displayString("Wrong Pass");
			_delay_ms(500); //new button every 500ms
			LCD_clearScreen();
			LCD_displayString("Re-enter Pass:");
			LCD_goToRowColumn(1,0);
			UART_sendByte(CHECKPASSFLAG);
			UART_sendByte(CHECKPASSFLAG);
			sendFiveCharacters();

		}




	}
}

