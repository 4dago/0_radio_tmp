/*
 * main.c
 *
 *  Created on: 10 mar 2018
 *      Author: dago
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>
// #include <stdio.h>

#include "Si4703/Si4703.h"
#include "I2C_TWI/i2c_master.h"
#include "UART/mkuart.h"
#include "ssd1306/ssd1306.h"

volatile uint8_t licznik;
volatile uint8_t RDStest;



void pokaz_rejestry (uint8_t start, uint8_t stop, uint16_t * bufor);
void pokazHz(int x, int y, int data, uint8_t txt_size, uint8_t color, uint8_t bg);


const uint8_t GLOSNIK8x16r[] PROGMEM = {
	0x0008, 0x0010,    // size
	0xE0,0x82,0x86,0x0C,0x1A,0x96,0xEE,0x9E,0xFE,0xFE,0x9E,0x1E,0x0E,0x86,0x82,0xE0,
};

const uint8_t GLOSNIK8x16[] PROGMEM = {
	0x0008, 0x0010,    // size
	0x00,0x02,0x06,0x0C,0x1A,0x96,0xEE,0x9E,0xFE,0xFE,0x9E,0x1E,0x0E,0x06,0x02,0x00,
};

const uint8_t RAMKAP3x16[] PROGMEM = {
	0x0003, 0x0010,    // size
	0xFF,0x3F,0x3F,0x1F,0x1F,0x1F,0x3F,0x3F,0x3F,0x3F,0x1F,0x1F,0x1F,0x3F,0x3F,0xFF,
};

const uint8_t RADIO16x16[] PROGMEM = {
	0x0010, 0x0010,    // size
	0x2F,0xE8,0x20,0x08,0x7F,0xFC,0xFF,0xFE,0xC0,0x06,0xC8,0x06,0xC8,0x06,0xFF,0xFE,
	0xC3,0xCE,0xFF,0x86,0xC3,0x02,0xFF,0x02,0xC3,0x86,0xFF,0xCE,0xFF,0xFE,0x00,0x00,
};

const uint8_t RSSI19x16[] PROGMEM = {
	0x0013, 0x0010,    // size
	0xFF,0xFF,0xFF,0xC4,0x21,0x7F,0xD5,0xAD,0x7F,0xC4,0xE7,0x7F,0xCF,0x39,0x7F,0xD5,
	0xAD,0x7F,0xD4,0x21,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};




int main(void) {

	// ******************* Przerwanie co 10ms z timera 2 ************************
	TCCR2A  	|= (1<<WGM21);							// tryb CTC s. 205
	TCCR2B		|= (1<<CS22) | (1<<CS21) | (1<<CS20);	// preskaler 1024

	OCR2A		= (F_CPU / 1024UL / 300UL) - 1; 		// przerwanie co 5ms (100Hz)
	TIMSK2		|= (1<<OCIE2A);							// Odblokowanie przerwania CompareMatch

	// *************************** inicjalizacje ********************************

	USART_Init(__UBRR);								// Inicjalizacja UART
	i2c_init(I2CBITRATE);								//inicjalizacja i2c
	si4703_init();
    SSD1306_init( SSD1306_SWITCHCAPVCC, REFRESH_MID );

	// *************************** przygotowania ********************************

    sei();												// globalne przerwainia
	fm_setChannel(967);
	fm_setVolume(5);

	pokaz_rejestry(0x00, 0x0f, si4703_registers);

	char rdsname[9] = "HELLO :)";
	char rdsrt[65] = "*** Radyjo *** MADZINKI ***";
	DDRB |= (1 << PB5);	// LED testowy



	// *************************** warstwa 1 ********************************

    SSD1306_cls(); 									// czyszczenie zawertosci bufora
	SSD1306_drawKnownBitmap_P(0,page0*8,RADIO16x16,1); // rysuj radio
    SSD1306_drawKnownBitmap_P(0,page6*8-1,GLOSNIK8x16,1); // rysuj glosnik
    SSD1306_drawKnownBitmap_P(41,page6*8,RSSI19x16,1); // rysuj bitmapehar

    SSD1306_display();

	while (1) {

/*
		if (1 == RDS_RTready) {
		for (int i = 130; i>-606;i-- ) {
			SSD1306_puts(i,page3*8,rdsrt,1,1,0);
			SSD1306_refreshPages(page3,3,0,127);
		}
		// *************** test 3 ************** DZIAŁA !
		// https://www.avrfreaks.net/forum/ssd1306-lcd-initialization-commands

		SSD1306_cmd(SSD1306_COLUMNADDR); 					// 0x21 COMMAND
		SSD1306_cmd(0); 									// Column start address
		SSD1306_cmd(SSD1306_WIDTH-1); 						// Column end address

		SSD1306_cmd(SSD1306_PAGEADDR); 					// 0x22 COMMAND
		SSD1306_cmd(0); 									// Start Page address
		SSD1306_cmd((SSD1306_HEIGHT/8)-1);					// End Page address

		RDS_RTready=0;
		}
*/

		if (1 == RDS_PSready) {
			SSD1306_puts(20, page0 * 8, rdsname, 2, 1, 0);
//			SSD1306_display();
			RDS_PSready = 0;
		}

		if (1 == RDS_CTready) {
			SSD1306_put_int(100, page4 * 8, godziny + offsetutc / 2, 1, 1, 0);
			SSD1306_put_int(100, page5 * 8, minuty, 1, 1, 0);
//			SSD1306_display();
//			uart_putint(godziny+offsetutc/2, 10);
//			uart_puts(":");
//			uart_putint(minuty, 10);
//			uart_puts(" , offset: ");
//			uart_putint(offsetutc, 10);
			RDS_CTready = 0;
		}

//		if (1 == RDS_RTready) {
//			for (int i = 100; i > -100; i--) {
//				SSD1306_puts(i, page2 * 8, rdsrt, 1, 1, 0);
//				SSD1306_refreshPages(page2, 1, 0, 128);
//			}
//			RDS_RTready = 0;
//		}
//		}
//		PORTB ^= (1 << PB5);
//		_delay_ms(3000);
//		kierunek = uart_getc();

			if (RDStest) {

				static int i;

				if (i<-387) {i = 128; RDS_RTready = 0;}
				i--;

//				if (1 == RDS_RTready) {
	//			for (i = 130; i>-606;i-- ) {
					SSD1306_puts(i,page3*8,rdsrt,1,1,0);
					SSD1306_refreshPages(page3,3,0,127);

	//			}
				// *************** test 3 ************** DZIAŁA !
				// https://www.avrfreaks.net/forum/ssd1306-lcd-initialization-commands

				SSD1306_cmd(SSD1306_COLUMNADDR); 					// 0x21 COMMAND
				SSD1306_cmd(0); 									// Column start address
				SSD1306_cmd(SSD1306_WIDTH-1); 						// Column end address

				SSD1306_cmd(SSD1306_PAGEADDR); 					// 0x22 COMMAND
				SSD1306_cmd(0); 									// Start Page address
				SSD1306_cmd((SSD1306_HEIGHT/8)-1);					// End Page address

//				RDS_RTready=0;
//				}

				fm_readRDS(rdsname, rdsrt);
				fm_getRssi ();
			    SSD1306_puts(11,page6*8, glosnik,2,1,0);
			//		    SSD1306_drawKnownBitmap_P(34,page6*8-1,RAMKAP3x16,1); // rysuj bitmapehar

			    SSD1306_put_int(45,page7*8, rssi,1,0,1);

			    pokazHz(66,page6*8, fm_getChannel10x(),2,1,0);




			    SSD1306_display();
				RDStest = 0;
			}




		switch(uart_getc()) {
			case 'g':
				fm_seek(1);
				uart_putint(fm_getChannel10x(), 10);

				break;
			case 'd':
				fm_seek(0);
				uart_putint(fm_getChannel10x(), 10);

				break;
			case '+':
//				fm_readRDS(rdsdata, radiotext);
				fm_setVolume(glosnosc+1);
				uart_putint(glosnosc, 10);

				break;
			case '-':
//				fm_readRDS(rdsdata, radiotext);
				fm_setVolume(glosnosc-1);
				uart_putint(glosnosc, 10);
				break;
			case 'r':
//				uart_putint( fm_readRDS(rdsdata, radiotext), 10);
				uart_puts(rdsname);
				uart_puts("\n\r");
				uart_puts(rdsrt);
				uart_puts("\n\r");
				uart_putint(godziny+offsetutc/2, 10);
				uart_puts(":");
				uart_putint(minuty, 10);
				uart_puts(" , offset: ");
				uart_putint(offsetutc, 10);
				uart_puts("\n\r");
				break;
			case 's':
				pokaz_rejestry(0x00, 0x0f, si4703_registers);
				break;
		}




	}

}





void pokaz_rejestry (uint8_t start, uint8_t stop, uint16_t * bufor){
	uart_puts("\n\r");
	for (uint8_t i = start; i <= stop; i++){
		uart_puts("Rejestr ");
		uart_putint(i, 16);
		uart_puts(":  ");
		uart_putint(*bufor++, 2);
		uart_puts("\n\r");
	}
}





ISR (TIMER2_COMPA_vect) {

licznik++;
if (licznik % 10 == 0) RDStest = 1;
}






void pokazHz(int x, int y, int data, uint8_t txt_size, uint8_t color, uint8_t bg) {
	char buf[5];
	cursor_x = x;
	cursor_y = y;
	uint8_t cale = data/10;
	uint8_t reszta = data%10;
	if (cale<100) {
		buf[0]=' ';
		itoa(data, buf+1, 10);
	} else {
		itoa(data, buf, 10);
	}
//	buf[3] = ',';
	buf[3] = '0' + reszta;
	buf[4] = '\0';

	for (uint8_t i = 0; i < 3; i++ ) {
			SSD1306_drawChar(cursor_x, cursor_y, buf[i], color, bg, txt_size);
			cursor_x += txt_size * 6;														// szerokość znaku
	}
	SSD1306_drawChar(cursor_x, cursor_y, buf[3], 1, 0, 1);
	SSD1306_drawFastVLine(cursor_x,cursor_y+8,8, 1);
	SSD1306_puts(cursor_x+1, cursor_y+8,"MHz",1,0,1);


//	SSD1306_puts(x, y, buf, txt_size, color, bg);																// szerokość znaku
}

