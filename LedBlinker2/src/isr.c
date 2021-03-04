/*
 * isr.c
 *
 * Created: 2/25/2021 10:22:15 AM
 *  Author: jacob
 */ 

#include <stdio.h>
#include <atmel_start_pins.h>
#include <util/delay.h>


ISR(PORTC_PORT_vect)
{
	printf("PORTC ISR\r\n");
	
	if (PORTC.INTFLAGS & 0x10)
	{
		printf("PORTC ISR\r\n");
		
		LED_Pin_set_level(0);
		_delay_ms(2000);
		LED_Pin_set_level(1);
		
		PORTC.INTFLAGS |= 0x08;
	}
}
