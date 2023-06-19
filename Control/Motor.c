#include "avr/io.h" /* To use the ADC Registers */
#include "motor.h"
#include "common_macros.h" /* To use the macros like SET_BIT */
#include "gpio.h"

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
void PWM_Timer0_Start(uint8 duty_cycle)
{
	TCNT0 = 0; //Set Timer Initial value
	OCR0  = duty_cycle; // Set Compare Value
	GPIO_setupPinDirection(MOTOR_ENABLE_PORT_ID,MOTOR_ENABLE_PIN_ID,PIN_OUTPUT); //set PB3/OC0 as output pin --> pin where the PWM signal is generated from MC.
	/* Configure timer control register
	 * 1. Fast PWM mode FOC0=0
	 * 2. Fast PWM Mode WGM01=1 & WGM00=1
	 * 3. Clear OC0 when match occurs (non inverted mode) COM00=0 & COM01=1
	 * 4. clock = F_CPU/8 CS00=0 CS01=1 CS02=0
	 */
	TCCR0 = (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS01);
}

void DcMotor_init(void) {
	GPIO_setupPinDirection(MOTOR_INPUT_PORT_ID,MOTOR_INPUT1_PIN_ID,PIN_OUTPUT);
	GPIO_setupPinDirection(MOTOR_INPUT_PORT_ID,MOTOR_INPUT2_PIN_ID,PIN_OUTPUT);
	// Stop the DC Motor at the beginning
	GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT1_PIN_ID,LOGIC_LOW);
	GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT2_PIN_ID,LOGIC_LOW);
}

void DcMotor_Rotate(DcMotor_State state, uint8 speed)
{
	switch (state) {
	case STOP:
		GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT1_PIN_ID,LOGIC_LOW);
		GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT2_PIN_ID,LOGIC_LOW);
		break;
	case CW:
		GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT1_PIN_ID,LOGIC_LOW);
		GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT2_PIN_ID,LOGIC_HIGH);
		break;
	case ACW:
		GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT1_PIN_ID,LOGIC_HIGH);
		GPIO_writePin(MOTOR_INPUT_PORT_ID,MOTOR_INPUT2_PIN_ID,LOGIC_HIGH);
		break;
	}
	PWM_Timer0_Start(((float)PWM_MAX_COUNT/100)*speed);
}
