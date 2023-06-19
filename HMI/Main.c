/*******************************************************************************
 *                                   INCLUDES                                  *
 *******************************************************************************/
#include "lcd.h"
#include "keypad.h"
#include "uart.h"
#include "timer0.h"
#include <util/delay.h>

#define F_CPU 1000000
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
volatile uint8 g_recievedFlag=0 ;
uint8 g_entryCount=0;
uint8 g_passwordEntry=0 ;
uint8 g_key;
uint8 g_option;

/*******************************************************************************
 *                         Functions Definitions                               *
 *******************************************************************************/
void setPassword(void){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0,"Enter New");
	LCD_displayStringRowColumn(1,0,"Password: ");
	UART_sendByte(FIRST_PASS_ENTRY) ;
	while(g_entryCount==0) {
		g_key=KEYPAD_getPressedKey();
		if (g_key>=0 && g_key<=9) {
			UART_sendByte(g_key);
			LCD_displayString("*"); /* display the pressed keypad switch as '*' */
		}
		else if(g_key == '=') {
			UART_sendByte(DONE) ;
			g_entryCount++;
		}
		_delay_ms(1000);
	}
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0,"Re-Enter");
	LCD_displayStringRowColumn(1,0,"Password: ");
	UART_sendByte(SECOND_PASS_ENTRY);
	while (g_entryCount==1) {
		g_key=KEYPAD_getPressedKey();
		if (g_key>=0 && g_key<=9) {
			UART_sendByte(g_key) ;
			LCD_displayString("*"); /* display the pressed keypad switch as '*' */
		}
		else if(g_key == '=') {
			UART_sendByte(DONE) ;
			g_entryCount++;
		}
		_delay_ms(1000);
	}
}

void enterPassword(void) {
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0,"Enter Password:");
	LCD_moveCursor(1,0);
	UART_sendByte(CHECK_PASSWORD) ;
	while(g_passwordEntry==0) {
		g_key=KEYPAD_getPressedKey();
		if (g_key>=0 && g_key<=9) {
			UART_sendByte(g_key);
			LCD_displayString("*"); /* display the pressed keypad switch as * */
		}
		else if(g_key == '=') {
			UART_sendByte(DONE) ;
			g_passwordEntry++;
		}
		_delay_ms(1000);
	}
}

void mainMenu(void) {
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0,"+: Change Pass");
	LCD_displayStringRowColumn(1,0,"-: Open Door") ;
	g_key = KEYPAD_getPressedKey();
	if (g_key =='+') {
		enterPassword();
		g_option='+';
	}
	else if(g_key == '-') {
		enterPassword();
		g_option='-';
	}
	_delay_ms(1000);
}

void main(void) {
	SREG  |= (1<<7);
	UART_ConfigType UART_Config ={EIGHT_BITS,DISABLE_PARITY,ONE_BIT,9600};
	UART_init(&UART_Config);
	LCD_init();
	/* UART_sendByte(HMI_READY);
	while(UART_recieveByte() != CONTROL_READY);
	if(UART_recieveByte() == PASSWORD_NOT_SET)*/
	setPassword();

	while(1) {
		//UART_sendByte(HMI_READY);
		g_entryCount=0;
		g_passwordEntry=0;
		g_recievedValue=UART_recieveByte();
		switch(g_recievedValue) {
		case UNMATCHED:
			g_recievedValue=0;
			LCD_clearScreen();
			LCD_displayStringRowColumn(0,0,"Passwords does");
			LCD_displayStringRowColumn(1,0,"not match...");
			_delay_ms(1000);
			setPassword();
			break;
		case WRONG_PASSWORD:
			g_recievedValue=0;
			LCD_clearScreen();
			LCD_displayStringRowColumn(0,0,"Wrong password");
			_delay_ms(1000);
			enterPassword();
			break;
		case RIGHT_PASSWORD:
			g_recievedValue=0;
			if(g_option=='-')
				UART_sendByte(UNLOCK_DOOR) ;
			else if(g_option=='*')
				setPassword();
			break;
		case OPENING:
			g_recievedValue=0;
			LCD_clearScreen();
			LCD_displayStringRowColumn(0,0,"OPENING DOOR...");
			break;
		case CLOSING:
			g_recievedValue=0;
			LCD_clearScreen();
			LCD_displayStringRowColumn(0,0,"CLOSING DOOR...");
			break;
		case ERROR_SCREEN :
			g_recievedValue=0;
			LCD_clearScreen();
			LCD_displayStringRowColumn(0,0,"ERROR!!!");
			break ;
		case DISPLAY_OPTIONS:
			g_recievedValue=0;
			mainMenu();
			break;
		default:
			break;
		}
	}
}

