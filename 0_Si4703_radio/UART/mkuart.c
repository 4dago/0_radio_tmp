/*
 * mkuart.c
 *
 *  Created on: 2010-09-04
 *       Autor: Mirosław Kardaś
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "mkuart.h"


// definiujemy w końcu nasz bufor UART_RxBuf
volatile char UART_RxBuf[UART_RX_BUF_SIZE];
// definiujemy indeksy określające ilość danych w buforze
volatile uint8_t UART_RxHead; 									// indeks oznaczający głowę węża
volatile uint8_t UART_RxTail; 									// indeks oznaczający ogon węża


volatile char UART_TxBuf[UART_TX_BUF_SIZE];					// definiujemy w końcu nasz bufor UART_TxBuf
// definiujemy indeksy określające ilość danych w buforze
volatile uint8_t UART_TxHead; 									// indeks oznaczający głowę węża
volatile uint8_t UART_TxTail; 									// indeks oznaczający ogon węża


// Inicjalizacja + włączenie przerwania od rejestru przychodzącego
void USART_Init(uint16_t baud) {

	/* Ustawienie prędkości */
	UBRR0H = (uint8_t) (baud >> 8);								// 4 z 12 bitów do rejestru górnego
	UBRR0L = (uint8_t) baud;									// pozostałe 8 z 12 bitów do rejestru dolnego

	/* Załączenie nadajnika I odbiornika */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	/* Ustawienie format ramki: 8bitów danych, 1 bit stopu
	UCSRC = (1<<URSEL)|(3<<UCSZ0); z innego procka ...
	********** atmega 328: UCSR0C domyślnie ustawiony na asynchroniczne 8n1 *******************/

#ifdef UART_DE_PORT 											// jeśli korzystamy z interefejsu RS485
	// inicjalizujemy lini� steruj�c� nadajnikiem
	UART_DE_DIR |= UART_DE_BIT;
	UART_DE_ODBIERANIE;
#endif
#ifdef UART_DE_PORT												// jeśli korzystamy z interefejsu RS485
	UCSRB |= (1<<RXEN)|(1<<TXEN)|(1<<RXCIE)|(1<<TXCIE);			// załączamy dodatkowe przerwanie TXCIE
#else
	// jeśli nie  korzystamy z interefejsu RS485
	UCSR0B |= (1 << RXCIE0);									//załaczenie przerwania od bufora rejestru ** przychodzącego **
}
#endif
#ifdef UART_DE_PORT
// procedura obs�ugi przerwania Tx Complete, gdy zostanie op�niony UDR
// kompilacja gdy u�ywamy RS485
ISR( USART_TXC_vect ) {
	UART_DE_PORT &= ~UART_DE_BIT;	// zablokuj nadajnik RS485
}
#endif


// definiujemy funkcję dodającą jeden bajt do bufora cyklicznego
void uart_putc(char data) {

	uint8_t tmp_head;

	tmp_head = (UART_TxHead + 1) & UART_TX_BUF_MASK;				// 0+1 & (ob00001111) ->index+1 lub początek (0)
	while (tmp_head == UART_TxTail) {								// pętla oczekuje jeśeli brak miejsca w buforze cyklicznym na kolejne znaki
	}
	UART_TxBuf[tmp_head] = data;
	UART_TxHead = tmp_head;
	UCSR0B |= (1 << UDRIE0);										// włączenie przerwania wolnego bufora ** nadawczego **, przerwanie wyśle dane z bufora
}


// procedura obsługi przerwania nadawczego, pobierająca dane z bufora cyklicznego
ISR( USART_UDRE_vect) {

	if (UART_TxHead != UART_TxTail) {								// sprawdzamy czy indeksy są równe
		UART_TxTail = (UART_TxTail + 1) & UART_TX_BUF_MASK;			// nowy indeks ogona węża (może się zr,ownać z głową)
		UDR0 = UART_TxBuf[UART_TxTail];								// łąduje bajt do rejestru UDR0 ** wyjściowego **
	} else {
		UCSR0B &= ~(1 << UDRIE0);									// gdy bufor pusty, wyłączamy przerwanie od bufora ** nadawczego **
	}
}


void uart_puts(char *s) {											// wysyła łańcuch z pamięci RAM na UART

	register char c;

	while ((c = *s++))	uart_putc(c);								// dopóki nie napotkasz 0 wysyłaj znak
}

// wysyła na port szeregowy liczbę jako tekst @radix-format (2- binarny, 8, 10, 16...
void uart_putint(int value, int radix) {

	char string[17];												// bufor na wynik funkcji itoa
	itoa(value, string, radix);										// konwersja value na ASCII
	uart_puts(string);												// wyślij string na port szeregowy
}


// definiujemy funkcję pobierającą jeden bajt z bufora cyklicznego
char uart_getc(void) {

    if ( UART_RxHead == UART_RxTail ) return 0;										// sprawdzamy czy indeksy są równe (bufor zapełniony)
    UART_RxTail = (UART_RxTail + 1) & UART_RX_BUF_MASK;								// nowy indeks ogona (może się zrównać z głową)
    return UART_RxBuf[UART_RxTail];													// zwracamy bajt pobrany z bufora  jako rezultat funkcji
}


// definiujemy procedurę obsługi przerwania odbiorczego, zapisując dane do bufora cyklicznego
ISR( USART_RX_vect ) {

    uint8_t tmp_head;
    char data;

    data = UDR0; 																	//pobieramy natychmiast bajt danych z bufora sprzętowego
    tmp_head = ( UART_RxHead + 1) & UART_RX_BUF_MASK;								// obliczamy nowy indeks głowy węża
    if ( tmp_head == UART_RxTail ) {												// sprawdzamy, czy właśnie zacznie zjadać własnego ogona
    	// tutaj możemy w jakiś wygodny dla nas sposób obsłużyć  błąd spowodowany
    	// próbą nadpisania danych w buforze, mogłoby dojść do sytuacji gdzie
    	// nasz wą zacząłby zjadać własny ogon
    } else {
	UART_RxHead = tmp_head; 														// zapamiętujemy nowy indeks
	UART_RxBuf[tmp_head] = data; 													// wpisujemy odebrany bajt do bufora
    }
}

