#include <atmel_start.h>
#include <util/delay.h>

#include <state.h>
#include <radio.h>

enum STATE currentState;

int main(void)
{
	
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	//LED_Pin_set_level(1);
	Button_Pin_set_isc(PORT_ISC_FALLING_gc);
	
	currentState = INIT;
	uint8_t level = 0;
	/* Replace with your application code */
	while (1) {
		
		switch (currentState)
		{
			case INIT:
				printf("Hello World!\r\n");
				LED_Pin_set_level(1);
				_delay_ms(500);
				LED_Pin_set_level(0);
				_delay_ms(500);
				Button_Pin_set_level(level);
				printf("Level %u\r\n", level);
				level = !level;
				break;	
		}
		
		
		//SendData(NULL, 0);
	}
}
