#ifndef TIMER0_H_
#define TIMER0_H_

#include "std_types.h"
#include<avr/io.h>

typedef enum
{
	OVF,CMP
}timerMode;

typedef enum
{
	NO_CLOCK,NO_PRESCALER,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024
}Prescaler;

typedef struct
{
	timerMode mode;
	uint8 initialValue;
	uint8 compareValue;
	Prescaler prescaler;
}Timer0_ConfigType;

void Timer0_init(const Timer0_ConfigType * Config_Ptr);
void Timer0_setCallBack(void(*a_ptr)(void));
void Timer0_stop();
void Timer0_deinit(void);

#endif /* TIMER0_H_ */
