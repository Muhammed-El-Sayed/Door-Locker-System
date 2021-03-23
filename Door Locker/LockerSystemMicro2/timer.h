/******************************************************************************
 *
 * Module: Timer
 *
 * File Name: timer.h
 *
 * Description: Header file for the Timer AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#ifndef TIMER_H_
#define TIMER_H_

#include "micro_config.h"
#include "std_types.h"
#include "common_macros.h"

#define OC0 PB3
#define OC0REG DDRB

#define OC1A PD5
#define OC1B PD4
#define OC1AREG DDRD
#define OC1BREG DDRD

/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/

typedef enum
{
	TIMER0,TIMER1
}Timer_N0;

typedef enum
{
	A,B
}Timer1_Channel;


typedef enum
{
	NORMAL,COMPARE=2
}Timer0_Mode;

typedef enum
{
	DISCONNECT,TOGGLE,NON_INVERTING,INVERTING
}Compare_Match_Output_Mode;

typedef enum
{
	NO_CLOCK,F_CPU_CLOCK,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024
}Timer0_Clock;

typedef struct
{
	Timer_N0 TimerN0;
	Timer1_Channel Timer1Channel;
	Timer0_Mode TimerMode;
	uint16 TimerInitialValue;
	Timer0_Clock Clock;
	uint16 OutputCompareValue;
	Compare_Match_Output_Mode CompareMatchOutMode;

}TIMER_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

void Timer_init(const TIMER_ConfigType* Config_Ptr);
void Timer0_Overflow_ISR_setCallBack(void(*a_ptr)(void));
void Timer0_CTC_ISR_setCallBack(void(*a_ptr)(void));
void Timer1_Overflow_ISR_setCallBack(void(*a_ptr)(void));
void Timer1_COMPA_ISR_setCallBack(void(*a_ptr)(void));
void Timer1_COMPB_ISR_setCallBack(void(*a_ptr)(void));




#endif /* TIMER_H_ */
