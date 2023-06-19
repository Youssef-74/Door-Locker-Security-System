#ifndef MOTOR_H_
#define MOTOR_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
/* MOTOR HW Ports and Pins IDs */
#define MOTOR_INPUT_PORT_ID					PORTB_ID
#define MOTOR_INPUT1_PIN_ID					PIN0_ID
#define MOTOR_INPUT2_PIN_ID					PIN1_ID
#define MOTOR_ENABLE_PORT_ID				PORTB_ID
#define MOTOR_ENABLE_PIN_ID					PIN3_ID

#define PWM_MAX_COUNT					        255
/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/
typedef enum
{
	STOP,CW,ACW
}DcMotor_State;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Function responsible for for setup the direction for the two motor pins.
 */
void DcMotor_init(void);

/*
 * Description :
 * Function responsible for rotating the DC Motor CW or ACW or stop the motor.
 */
void DcMotor_Rotate(DcMotor_State state,uint8 speed);

#endif /* MOTOR_H_ */
