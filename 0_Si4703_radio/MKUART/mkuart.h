/*
 * mkuart.h
 *
 *  Created on: 2010-09-04
 *       Autor: Miros�aw Karda�
 */

#ifndef MKUART_H_
#define MKUART_H_


#define UART_BAUD 19200											// tu definiujemy interesującą nas prędkość
//#define __UBRR F_CPU/16/UART_BAUD-1  							// obliczamy UBRR dla U2X=0 (działa dla wybranych kwarców
//*** mirekk36.blogspot.com/2013/01/rs232-ubrr-jak-prawidowo-obliczac-trick.html
// dla okrągłych wartości kwarców:
#define __UBRR ((F_CPU + UART_BAUD * 8UL) / (16UL * UART_BAUD) -1)
// i jeszcze: https://www.mikrocontroller.net/topic/170617#1631916


// definicje na potrzeby RS485 odkomentować jeśli potrzeba
 //#define UART_DE_PORT PORTD
 //#define UART_DE_DIR DDRD
 //#define UART_DE_BIT (1<<PD2)

 //#define UART_DE_ODBIERANIE  UART_DE_PORT |= UART_DE_BIT
 //#define UART_DE_NADAWANIE  UART_DE_PORT &= ~UART_DE_BIT


#define UART_RX_BUF_SIZE 32 									// bufor odbiorczy 32 bajtÓw *** musi być potęgą 2 aby działał cyklicznie 32/64/128
#define UART_RX_BUF_MASK ( UART_RX_BUF_SIZE - 1)				// definiujemy maskę dla naszego bufora

#define UART_TX_BUF_SIZE 16 									// definiujemy bufor nadawania o rozmiarze 16 bajtów jw.
#define UART_TX_BUF_MASK ( UART_TX_BUF_SIZE - 1)				// definiujemy maskę dla naszego bufora


// deklaracje funkcji publicznych

void USART_Init( uint16_t baud );
char uart_getc(void);
void uart_putc( char data );
void uart_puts(char *s);
void uart_putint(int value, int radix);

#endif /* MKUART_H_ */
