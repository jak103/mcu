#include <atmel_start.h>
#include <util/delay.h>

#include <state.h>
#include <radio.h>
#include <stdio.h>

enum STATE currentState;

int main(void)
{
	
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	Button_Pin_set_isc(PORT_ISC_FALLING_gc);
	
	currentState = INIT;
	
	/* Replace with your application code */
	while (1) {
		
		switch (currentState)
		{
			case INIT:
				printf("Init\r\n");
				LED_Pin_set_level(1);
				currentState = LISTEN;
				break;
	
			case LISTEN:
				break;	
		}
	}
}
