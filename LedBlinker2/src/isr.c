/*
 * isr.c
 *
 * Created: 2/25/2021 10:22:15 AM
 *  Author: jacob
 */ 

#include <stdio.h>
#include <atmel_start_pins.h>

ISR(PORTC_PORT_vect)
{
	printf("PORTC ISR\r\n");
	
	if (PORTC.INTFLAGS & 0x08)
	{
		printf("PORTC ISR\r\n");
		//LED_Pin_set_level(1);
		//_delay_ms(500);
		//LED_Pin_set_level(0);
		//_delay_ms(500);
		PORTC.INTFLAGS |= 0x08;
	}
	
	//PORTC.INTFLAGS |= 0xFF;
}
