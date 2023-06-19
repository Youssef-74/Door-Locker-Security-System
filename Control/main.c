#define F_CPU 1000000

/*******************************************************************************
 *                                   INCLUDES                                  *
 *******************************************************************************/
#include "external_eeprom.h"
#include "uart.h"
#include "timer0.h"
#include "motor.h"
#include "common_macros.h"
#include <util/delay.h>

/*******************************************************************************
 *                                  Definitions                                *
 *******************************************************************************/
#define HMI_READY 					0x10
#define CONTROL_READY				0x20
#define DONE 						0x00
#define WRONG_PASSWORD 				0x30
#define RIGHT_PASSWORD 				0x31
#define UNMATCHED 					0x32
#define PASSWORD_SET 				0x33
#define PASSWORD_NOT_SET			0x34
#define SAVE_PASSWORD 				0x35
#define FIRST_PASS_ENTRY 			0x01
#define SECOND_PASS_ENTRY 			0x02
#define CHECK_PASSWORD 				0x03
#define DISPLAY_OPTIONS 			0x04
#define PASS_END_BYTE 				0x05
#define ERROR_SCREEN				0x06
#define UNLOCK_DOOR 				0x40
#define OPENING 					0x41
#define CLOSING 					0x42

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/
volatile uint8 g_recievedValue=0;
volatile uint8 g_recievedFlag=0;
uint16 g_tick=0;
uint8 g_trialNumber=0;
uint8 g_entryCount=0;
uint8 g_passwordEntry=0 ;
uint8 g_key;
uint8 g_keyOption;
uint8 savedPassword[10];

Timer0_ConfigType timer0_config= {OVF,0,0,F_CPU_1024};

/*******************************************************************************
 *                         Functions Definitions                               *
 *******************************************************************************/
void g_tickCounterMotor(void) {
	g_tick++;
	if(g_tick==458)
		Timer0_deinit();
}

void g_tickCounterBuzzer(void) {
	g_tick++;
	if(g_tick==1831)
		Timer0_deinit();
}

void savePassword (uint8* password) {
	uint8 i=0;
	uint16 address=0x0311;
	while(password[i] != PASS_END_BYTE) {
		savedPassword[i]=password[i];
		EEPROM_writeByte(address,password[i]);
		_delay_ms(50);
		address++;
		i++;
	}
	EEPROM_writeByte(address,PASS_END_BYTE);
	UART_sendByte(DISPLAY_OPTIONS);
}

uint8 passCompare(uint8* firstPassword, uint8* secondPassword) {
	uint8 passError=0;
	uint8 i=0;
	while(firstPassword[i] != PASS_END_BYTE)  {
		if (firstPassword[i] != secondPassword[i]) {
			passError=1;
			break;
		}
		i++;
	}
	if(secondPassword[i] != PASS_END_BYTE)
		passError=1;
	return passError;
}

void buzzerOff(void) {
	CLEAR_BIT(PORTD,PD2);
	UART_sendByte(DISPLAY_OPTIONS);
}

void checkPassword (uint8* password) {
	uint16 address=0x0311;
	/*uint8 i=0;
	do {
		EEPROM_readByte(address,savedPassword+i);
		_delay_ms(10);
		address++;
		i++;
	}
	while(savedPassword[i-1] != PASS_END_BYTE);*/
	if(passCompare(password,savedPassword)==0) {
		g_trialNumber=0;
		UART_sendByte(RIGHT_PASSWORD);
	}
	else {
		g_trialNumber++ ;
		if (g_trialNumber==3){
			UART_sendByte(ERROR_SCREEN);
			SET_BIT(PORTD,PD2) ; // Activate Buzzer
			while(g_tick < 1831); // 1 minute
			g_tick=0;
			buzzerOff();
		}
		else UART_sendByte(WRONG_PASSWORD);
	}
}

void main(void) {
	SREG  |= (1<<7);
	g_recievedFlag=(UCSRA & 0x80)>>7;
	UART_ConfigType UART_Config ={EIGHT_BITS,DISABLE_PARITY,ONE_BIT,9600};
	UART_init(&UART_Config);
	SET_BIT(DDRD,PD2); /* Configure PD3 as output pin for buzzer */
	DcMotor_init();

	uint8 i,j,k; /*counting indexes*/
	uint8 receivedPass1[10],receivedPass2[10],receivedPass3[10],EEPROM_Data;

	/*while(UART_recieveByte() != HMI_READY);
	UART_sendByte(CONTROL_READY);
	if(EEPROM_readByte(0x000F,&EEPROM_Data) != PASSWORD_SET) {
		UART_sendByte(PASSWORD_NOT_SET);
		EEPROM_writeByte(0x000F,PASSWORD_SET);
	}*/

	while(1) {
		i=0; j=0; k=0;
		/*while(UART_recieveByte()!= HMI_READY);
		UART_sendByte(CONTROL_READY);*/
		g_recievedValue=UART_recieveByte();
		switch(g_recievedValue) {
		case FIRST_PASS_ENTRY:
			g_recievedValue=0;
			g_recievedFlag=0;
			g_recievedValue=0;
			do {
				g_recievedValue=UART_recieveByte();
				if((g_recievedFlag ==1) && (g_recievedValue != DONE)) {
					receivedPass1[i]=g_recievedValue ;
					i++;
					g_recievedFlag=0;
				}
			}
			while(g_recievedValue != DONE);
			receivedPass1[i]=PASS_END_BYTE;
			break;
		case SECOND_PASS_ENTRY:
			g_recievedValue=0;
			do {
				g_recievedValue=UART_recieveByte();
				if((g_recievedFlag==1) && (g_recievedValue != DONE)) {
					receivedPass2[j]=g_recievedValue ;
					j++;
					g_recievedFlag=0;
				}
			}
			while(g_recievedValue != DONE);
			receivedPass2[j]=PASS_END_BYTE;
			if((passCompare(receivedPass1,receivedPass2))==0)
				savePassword(receivedPass1);
			else UART_sendByte(UNMATCHED);
			break;
		case CHECK_PASSWORD:
			g_recievedValue=0;
			do {
				g_recievedValue=UART_recieveByte();
				if((g_recievedFlag==1) && (g_recievedValue != DONE)) {
					receivedPass3[k]=g_recievedValue ;
					k++;
					g_recievedFlag=0;
				}
			}
			while(g_recievedValue != DONE);
			receivedPass3[k]=PASS_END_BYTE;
			checkPassword(receivedPass3);
			break;
		case UNLOCK_DOOR:
			g_recievedValue=0;
			Timer0_init(&timer0_config);
			Timer0_setCallBack(g_tickCounterMotor);
			DcMotor_Rotate(CW,100);
			while(g_tick<485); // 15 sec
			DcMotor_Rotate(STOP,0);
			while(g_tick<549); // 3 sec
			DcMotor_Rotate(ACW,100);
			while(g_tick<1007); // 15 sec
			DcMotor_Rotate(STOP,0);
			g_tick=0;
			UART_sendByte(DISPLAY_OPTIONS);
			break;
		default:
			break;
		}
	}
}
