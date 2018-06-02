/*
 * buttons.h
 *
 *  Created on: 2 cze 2018
 *      Author: dago
 */

#ifndef BUTTONS_BUTTONS_H_
#define BUTTONS_BUTTONS_H_

//----------------------------------------------------------------------------------------
//
//		Ustawienia sprzętowe przycisków
//
//----------------------------------------------------------------------------------------
#define strojenie (1<<PB0)
#define vol_up (1<<PB0)





typedef struct {
 volatile uint8_t *KPIN;			// port przycisku
 uint8_t key_mask;					// pin przycisku
 uint8_t wait_time_s;				// długie naciśnięcie
 void (*kfun1)(void);				// 1. funkcja obsługi
 void (*kfun2)(void);				// 2. funkcja obsługi
 uint8_t klock;						// flaga blokady
 uint8_t flag;						// dodatkowa flaga
} TBUTTON;














#endif /* BUTTONS_BUTTONS_H_ */
