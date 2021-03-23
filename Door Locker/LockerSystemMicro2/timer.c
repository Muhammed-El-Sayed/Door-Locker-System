/******************************************************************************
 *
 * Module: Timer
 *
 * File Name: timer.c
 *
 * Description: Source file for the Timer AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#include "timer.h"

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/


/* Global variables to hold the address of the call back function in the application */
static volatile void (*g_callBackPtrTimer0OVF)(void) = NULL_PTR;
static volatile void (*g_callBackPtrTimer0CTC)(void) = NULL_PTR;
static volatile void (*g_callBackPtrTimer1OVF)(void) = NULL_PTR;
static volatile void (*g_callBackPtrTimer1COMPA)(void) = NULL_PTR;
static volatile void (*g_callBackPtrTimer1COMPB)(void) = NULL_PTR;
uint8 g_counter=0;
uint8 g_counter2=0;


/*******************************************************************************
 *                       Interrupt Service Routines                            *
 *******************************************************************************/

ISR(TIMER0_OVF_vect)
{

	if( (g_callBackPtrTimer0OVF != NULL_PTR)  )
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtrTimer0OVF)(); /* another method to call the function using pointer to function g_callBackPtr(); */

	}
}

ISR(TIMER0_COMP_vect)
{
	if(g_callBackPtrTimer0CTC != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtrTimer0CTC)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

ISR(TIMER1_OVF_vect)
{
	++g_counter;
	/*Timer1 overflow is 8.4sec so if statement will be executed after approximately 1 min*/
	if( (g_callBackPtrTimer1OVF != NULL_PTR) && (g_counter==7))
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtrTimer1OVF)(); /* another method to call the function using pointer to function g_callBackPtr(); */
		g_counter=0;
	}
}

ISR(TIMER1_COMPA_vect)
{
	++g_counter2;
	/*Timer1 compare value is 0XFFFF so ISR will be executed each 8.4sec so IF statement
	 *  will be executed after approximately 15 sec*/
	if(g_callBackPtrTimer1COMPA != NULL_PTR && (g_counter2==2))
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtrTimer1COMPA)(); /* another method to call the function using pointer to function g_callBackPtr(); */
		g_counter2=0;
	}
}

ISR(TIMER1_COMPB_vect)
{
	if(g_callBackPtrTimer1COMPB != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtrTimer1COMPB)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}



/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

void Timer_init(const TIMER_ConfigType* Config_Ptr)
{

	if((Config_Ptr->TimerN0) == TIMER0){

		if( (Config_Ptr->TimerMode) == NORMAL )
		{
			TCNT0 = Config_Ptr->TimerInitialValue; //Set Timer initial value
			TIMSK |= (1<<TOIE0); // Enable Timer0 Overflow Interrupt
			/* Configure the timer control register
			 * 1. Non PWM mode FOC0=1
			 * 2. Normal Mode WGM01=0 & WGM00=0
			 * 3. OC0 disconnected COM00=0 & COM01=0
			 * 4. Clock Bits CS00 CS01 CS02
			 */
			TCCR0 = (1<<FOC0) | (TCCR0 & 0xF8) | (Config_Ptr->Clock);
		}
		else if( (Config_Ptr->TimerMode) == COMPARE )
		{
			TCNT0 = Config_Ptr->TimerInitialValue; //Set Timer initial value
			OCR0 = Config_Ptr->OutputCompareValue; //Set Output Compare value
			TIMSK |= (1<<OCIE0); // Enable Timer0 Compare Match Interrupt

			/* Configure the timer control register
			 * 1. Non PWM mode FOC0=1
			 * 2. Compare Match Mode WGM01=1 & WGM00=0
			 * 3. Compare Match Output Mode Bits COM00 & COM01
			 * 4. Clock Bits CS00 CS01 CS02
			 */
			TCCR0 |= (1<<FOC0);
			TCCR0 |= 1<<WGM01;

			if((Config_Ptr->CompareMatchOutMode) == DISCONNECT )
			{
				TCCR0&= ~(1<<COM01) & ~(1<<COM00);
			}
			else if((Config_Ptr->CompareMatchOutMode) == TOGGLE )
			{
				TCCR0&= ~(1<<COM01);
				TCCR0|= 1<<COM00;
				SET_BIT(OC0REG,OC0);
			}
			else if((Config_Ptr->CompareMatchOutMode) == NON_INVERTING )
			{
				TCCR0&= ~(1<<COM00);
				TCCR0|= 1<<COM01;
				SET_BIT(OC0REG,OC0);
			}
			else if((Config_Ptr->CompareMatchOutMode) == INVERTING )
			{
				TCCR0|= (1<<COM00) | (1<<COM01);
				SET_BIT(OC0REG,OC0);

			}
			TCCR0 |= (TCCR0 & 0xF8) | (Config_Ptr->Clock);
		}


	}

	else if((Config_Ptr->TimerN0) == TIMER1){


		if( (Config_Ptr->TimerMode) == NORMAL ) //Mode0
		{
			TCNT1 = Config_Ptr->TimerInitialValue; //Set Timer initial value
			TIMSK |= (1<<TOIE1); // Enable Timer1 Overflow Interrupt
			/* Configure the timer control register
			 * 1. Non PWM mode FOC1A=1 & FOC1B=1
			 * 2. Normal Mode WGM13=0 & WGM12=0 & WGM11=0 & WGM10=0
			 * 3. OC1A & OC1B disconnected COM1A1=0 & COM1A0=0 & COM1B1=0 & COM1B0=0
			 * 4. Clock Bits CS00 CS01 CS02
			 */
			TCCR1A = (1<<FOC1A) | (1<<FOC1B);
			TCCR1B = (TCCR1B & 0xF8) | (Config_Ptr->Clock);
		}
		else if( (Config_Ptr->TimerMode) == COMPARE ) //Mode4
		{
			TCNT1 = Config_Ptr->TimerInitialValue; //Set Timer initial value

			if( (Config_Ptr->Timer1Channel) == A )
			{
				OCR1A = Config_Ptr->OutputCompareValue; //Set Output Compare value

				TIMSK |= (1<<OCIE1A); // Enable Timer1 Compare Match Interrupt
			}
			if( (Config_Ptr->Timer1Channel) == B )
			{
				OCR1B = Config_Ptr->OutputCompareValue; //Set Output Compare value

				TIMSK |= (1<<OCIE1B); // Enable Timer1 Compare Match Interrupt
			}


			/* Configure the timer control register
			 * 1. Non PWM mode FOC1A=1 & FOC1B=1
			 * 2. Compare Mode WGM13=0 & WGM12=1 & WGM11=0 & WGM10=0
			 * 3. OC1A connected & OC1B disconnected COM1A1 & COM1A0 & COM1B1 & COM1B0
			 * 4. Clock Bits CS00 CS01 CS02
			 */
			TCCR1A = (1<<FOC1A) | (1<<FOC1B);
			TCCR1B |= 1<<WGM12;
			if( (Config_Ptr->Timer1Channel) == A ){
				if((Config_Ptr->CompareMatchOutMode) == DISCONNECT )
				{
					TCCR1A&= ~(1<<COM1A1) & ~(1<<COM1A0);
				}
				else if((Config_Ptr->CompareMatchOutMode) == TOGGLE )
				{
					TCCR1A&= ~(1<<COM1A1);
					TCCR1A|= 1<<COM1A0;
					SET_BIT(OC1AREG,OC1A);
				}
				else if((Config_Ptr->CompareMatchOutMode) == NON_INVERTING )
				{
					TCCR1A&= ~(1<<COM1A0);
					TCCR1A|= 1<<COM1A1;
					SET_BIT(OC1AREG,OC1A);
				}
				else if((Config_Ptr->CompareMatchOutMode) == INVERTING )
				{
					TCCR1A|= (1<<COM1A0) | (1<<COM1A1);
					SET_BIT(OC1AREG,OC1A);

				}
			}
			else if ( (Config_Ptr->Timer1Channel) == B )
			{
				if((Config_Ptr->CompareMatchOutMode) == DISCONNECT )
				{
					TCCR1A&= ~(1<<COM1B1) & ~(1<<COM1B0);
				}
				else if((Config_Ptr->CompareMatchOutMode) == TOGGLE )
				{
					TCCR1A&= ~(1<<COM1B1);
					TCCR1A|= 1<<COM1B0;
					SET_BIT(OC1BREG,OC1B);
				}
				else if((Config_Ptr->OutputCompareValue) == NON_INVERTING )
				{
					TCCR1A&= ~(1<<COM1B0);
					TCCR1A|= 1<<COM1B1;
					SET_BIT(OC1BREG,OC1B);
				}
				else if((Config_Ptr->CompareMatchOutMode) == INVERTING )
				{
					TCCR1A|= (1<<COM1B0) | (1<<COM1B1);
					SET_BIT(OC1BREG,OC1B);

				}
			}
			TCCR1B |= (TCCR1B & 0xF8) | (Config_Ptr->Clock);
		}



	}
}

/*
 * Description: Function to set the Call Back function address.
 */
void Timer0_Overflow_ISR_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtrTimer0OVF = a_ptr;
}

void Timer0_CTC_ISR_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtrTimer0CTC = a_ptr;
}

void Timer1_Overflow_ISR_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtrTimer1OVF = a_ptr;
}

void Timer1_COMPA_ISR_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtrTimer1COMPA = a_ptr;
}

void Timer1_COMPB_ISR_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtrTimer1COMPB = a_ptr;
}


