/*
 * buttons.h
 *
 *  Created on: 2 cze 2018
 *      Author: dago
 */

#ifndef BUTTONS_BUTTONS_H_
#define BUTTONS_BUTTONS_H_

#include <avr/eeprom.h>
#include <avr/pgmspace.h>


//----------------------------------------------------------------------------------------
//
//		Ustawienia sprzętowe przycisków
//
//----------------------------------------------------------------------------------------

#define pinStrojenie 	(1<<PB0)
#define pinMemo1		(1<<PD7)
#define pinMemo2		(1<<PD6)


typedef struct {
	volatile uint8_t *KPIN;			// port przycisku
	uint8_t nr_guzika;					// guzik
	uint8_t key_mask;					// pin przycisku
	uint8_t wait_time_s;				// długie naciśnięcie
	void (*kfun1)(void);				// 1. funkcja obsługi
	void (*kfun2)(void);				// 2. funkcja obsługi
	uint8_t klock;						// flaga blokady
	uint8_t flag;						// dodatkowa flaga
} TBUTTON;



typedef struct {
	uint16_t Mem1;
	uint16_t Mem2;
} RADIOMEM;




extern RADIOMEM eem_memo EEMEM;		// dane w pamięci EEPROM
extern RADIOMEM ram_memo;			// dane w pamięci RAM

extern volatile uint16_t Timer1, Timer2;			/* timery programowe 100Hz */
extern volatile uint8_t Timer3, Timer4;  			// 100 *(10ms) * 120 sekund, 2';
extern TBUTTON Tstrojenie, Tmemo1, Tmemo2; 		// definicja KLAWISZA

extern volatile uint8_t guzik;





// funkcja obsługi pojedynczych klawiszy
extern void key_press(TBUTTON * btn);

// ******************* Przerwanie co 10ms z timera 2 ************************
extern void soft_timer_init(void);




void check_and_load(void);
void copy_eem_to_ram(void);
void copy_ram_to_eem(void);
//void copy_pgm_to_ram( void );
//void load_defaults( void );

#endif /* BUTTONS_BUTTONS_H_ */
