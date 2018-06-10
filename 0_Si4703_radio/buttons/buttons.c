/*
 * buttons.c
 *
 *  Created on: 2 cze 2018
 *      Author: dago
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "buttons.h"
#include "../Si4703/Si4703.h"



volatile uint16_t Timer1 = 12000, Timer2;			// timery programowe 100Hz 1-splash 2', 2-drgania
volatile uint8_t Timer3, Timer4;  					// 3-rds, 4-reszta danych
TBUTTON Tstrojenie, Tmemo1, Tmemo2;					// definicja KLAWISZA

RADIOMEM eem_memo EEMEM;							// dane w pamiêci EEPROM
RADIOMEM ram_memo;									// dane w pamiêci RAM

volatile uint8_t guzik;


// ******************* Przerwanie co 10ms z timera 2 ************************
void soft_timer_init(void) {
	TCCR2A  	|= (1<<WGM21);							// tryb CTC s. 205
	TCCR2B		|= (1<<CS22) | (1<<CS21) | (1<<CS20);	// preskaler 1024

	OCR2A		= (F_CPU / 1024UL / 156UL); 			// przerwanie co 10ms (100Hz)
	TIMSK2		|= (1<<OCIE2A);							// Odblokowanie przerwania CompareMatch
}



// funkcja obsługi pojedynczych klawiszy
void key_press(TBUTTON * btn) {
	register uint8_t key_press = (*btn->KPIN & btn->key_mask);

	if (!btn->klock && !key_press) {					// czy naciśnięty
		guzik = btn->nr_guzika;
		btn->klock = 1;
		btn->flag = 1;
		Timer2 = (btn->wait_time_s * 1000) / 10;

	} else if (btn->klock && key_press) {
		(btn->klock)++;									//odliczanie drgań
		if (!btn->klock) {								// wyzerował się licznik drgań
			if (btn->kfun1) btn->kfun1();				// akcja krótkie wciśnięcie
			guzik = 0;
			Timer2 = 0;
			btn->flag = 0;
		}
	} else if (btn->flag && !Timer2) {					// timer przytrzymania się wyzerował
		// reakcja na dłuższe wcinięcie klawisza
		if (btn->kfun2) btn->kfun2();
		btn->flag = 0;
	}
}


void copy_eem_to_ram( void ) {
	eeprom_read_block( &ram_memo, &eem_memo, sizeof(ram_memo) );
}


void copy_ram_to_eem( void ) {
	eeprom_write_block( &ram_memo, &eem_memo, sizeof(ram_memo) );
}

/*void copy_pgm_to_ram( void ) {
	memcpy_P( &ram_cfg, &pgm_cfg, sizeof(ram_cfg) );
}*/

/*void load_defaults( void ) {
	copy_pgm_to_ram();
	copy_ram_to_eem();
}*/


void check_and_load( void ) {
//	uint8_t i, len = sizeof( ram_memo );
//	uint8_t * ram_wsk = (uint8_t*) &ram_memo;

	copy_eem_to_ram();
	if (0xffff != ram_memo.Mem1) {			// jesli ustawiony
		fm_setChannel(ram_memo.Mem1);
		return;
	}
	if (0xffff != ram_memo.Mem2) {
		fm_setChannel(ram_memo.Mem2);
		return;
	}
	fm_setChannel(988);					// domyślnie pr3

/*	for(i=0; i<len; i++) {
		if( 0xff == *ram_wsk++ ) continue;
		break;
	}

	if( i == len ) {
//		load_defaults();
	}*/

}
