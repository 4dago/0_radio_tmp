/*
 * main.c
 *
 *  Created on: 10 mar 2018
 *      Author: dago
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Si4703/Si4703.h"
#include "I2C_TWI/i2c_master.h"
#include "UART/mkuart.h"


int main(void) {

	DDRB |= (1 << PB5);							// LED testowy

	USART_Init(__UBRR);						// Inicjalizacja UART

	si4703_init();
	i2c_init(I2CBITRATE);


	sei();										// globalne przerwainia


	while (1) {
		uart_putc('t');
		PORTB ^= (1 << PB5);
		_delay_ms(1000);

	}

}
