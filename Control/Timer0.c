#include "timer0.h"
#include<avr/interrupt.h>

static volatile void (*g_callBackPtr)(void) = NULL_PTR; // pointer to function 

ISR(TIMER0_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		(*g_callBackPtr)();
	}
}
ISR(TIMER0_COMP_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		(*g_callBackPtr)();
	}
}
void Timer0_init(const Timer0_ConfigType * Config_Ptr)
{
	TCNT0 = (Config_Ptr->initialValue) & 0XFF;
	TCCR0=0;
	TCCR0 = (TCCR0  & 0XF8) | (Config_Ptr->prescaler);
	switch(Config_Ptr->mode)
	{
	case OVF:
		TCCR0 |= (1<<FOC0);
		TIMSK |= (1<<TOIE0);
		break;
	case CMP:
		OCR0 = (Config_Ptr->compareValue)& 0XFF; // Set Compare Value
		TIMSK |= (1<<OCIE0); // Enable Timer0 Compare Interrupt
		TCCR0 |= (1<<FOC0) | (1<<WGM01);
		break;
	}
}
void Timer0_setCallBack(void(*a_ptr)(void))
{
	g_callBackPtr = a_ptr;
}
void Timer0_stop(void)
{
	TCCR0 &= 0xF8;
}
void Timer0_deinit(void)
{
	TCCR0= 0;
	TCNT0= 0;
	OCR0= 0;
	TIMSK= 0;
}
