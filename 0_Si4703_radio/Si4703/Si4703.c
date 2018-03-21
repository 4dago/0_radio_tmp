/*
 * Si4703.c
 *
 *  Created on: 10 mar 2018
 *      Author: dago
 */
#include <avr/io.h>
#include <util/delay.h>

#include "Si4703.h"
#include "../I2C_TWI/i2c_master.h"

//-------------------------------------------------------------------------
//								Zmienne globalne
//-------------------------------------------------------------------------
uint16_t si4703_registers[16]; 				//There are 16 registers, each 16 bits large
uint8_t reg_index;							// chyba nie musi być globalna
uint16_t ret;
uint8_t stereo;
uint16_t kanal10x;


/*************************************************************************
 Ustawia głośniść
 Ustawia głośność 0 - 15 (prosty wariant ustawiania)
  Parametry:	uint8_t volume - głośność
  todo: udoskonalić, uwzględnić wersje z VOLEXT 06h[8] bit, sprawdzić inne rejestry
*************************************************************************/
void fm_setVolume(uint8_t volume) {
  si4703_read1register(SYSCONFIG2);		//Read the current register set
  if(volume < 0) volume = 0;
  if (volume > 15) volume = 15;
  si4703_registers[SYSCONFIG2] &= 0xFFF0; 	//Clear volume bits
  si4703_registers[SYSCONFIG2] |= volume; 	//Set new volume
  si4703_writeBuffor(SYSCONFIG2,1);  		//Update
}


/*************************************************************************
 Szuka kanału
 Szuka stacji w górę lub w dół od bieżącej pozycji, aż do odlalezienia lub
 osiągnięcia końca zakresu,
 ustawia wskaźnik stereo,
  Parametry:	enum DIRECTION dir: down 0, up 1
  Zwraca: uint16_t 0 - limit zakresu, nic nie znalazł; kanał - jeśli znajdzie
  todo: nieładna, nie uwzględnia parametrów szukania, brak wersji szukania całego zakresu
*************************************************************************/
uint16_t fm_seek(enum DIRECTION dir) {
	si4703_readRegisters();
	si4703_registers[POWERCFG] &= ~(1 << SKMODE); 			//enable wrapping of frequencies
	if (dir == DOWN) {
//		printf("up\n");
		si4703_registers[POWERCFG] &= ~(1<<SEEKUP);
	} else {
//		printf("down\n");
		si4703_registers[POWERCFG] |= (1<<SEEKUP);
	}
	si4703_registers[POWERCFG] |= (1<<SEEK); 				//start seeking
	si4703_writeBuffor(POWERCFG,1);

	while (!(si4703_registers[STATUSRSSI] & (1<<STC))) {		//coś znalazł
		si4703_read1register(STATUSRSSI);
	} 															//seek complete!
	si4703_readRegisters();									// aktualizacja
	int valueSFBL = si4703_registers[STATUSRSSI] & (1<<SFBL); 	//Store the value of SFBL
	stereo = ( si4703_registers[STATUSRSSI] & (1<<ST));			// stereo?
	si4703_registers[POWERCFG] &= ~(1<<SEEK); 					//Clear the seek bit after seek has completed / zeruje SFBL
	si4703_writeRegisters2_7();

	  //Wait for the si4703 to clear the STC as well
	while(1) {
		si4703_readRegisters();
	    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
	}
	if(valueSFBL) { //The bit was set indicating we hit a band limit or failed to find a station
	    return(0);
	}
	return fm_getChannel10x();
}


/*************************************************************************
 Ustawia kanał
 Wpisuje do rejesru podany kanał
 Parametry:	uint16_t channel10x kanał - liczba całkowita
*************************************************************************/
void fm_setChannel(uint16_t channel10x) {
	//Freq(MHz) = 0.100(europa) * Channel + 87.5MHz
	//97.3 = 0.1 * Chan + 87.5
	//9.8 / 0.2 = 49
	uint16_t newChannel = channel10x * 10; 						//973 * 10 = 9730
	newChannel -= 8750; 										//9730 - 8750 = 980
	newChannel /= 10; 											//980 / 10 = 98

	//These steps come from AN230 page 20 rev 0.5
	si4703_read1register(CHANNEL);
	si4703_registers[CHANNEL] &= 0xFE00; 						//Clear out the channel bits
	si4703_registers[CHANNEL] |= newChannel; 					//Mask in the new channel
	si4703_registers[CHANNEL] |= (1 << TUNE); 					//Set the TUNE bit to start
	si4703_writeBuffor(CHANNEL,1);
	//delay(60); //Wait 60ms - you can use or skip this delay
	//Poll to see if STC is set
	while (1) {
		si4703_readRegisters();
		if ((si4703_registers[STATUSRSSI] & (1 << STC)) != 0)
			break; //Tuning complete!
	}
	si4703_registers[CHANNEL] &= ~(1 << TUNE); //Clear the tune after a tune has completed
	si4703_writeBuffor(CHANNEL,1);
	//Wait for the si4703 to clear the STC as well
	while (1) {
		si4703_readRegisters();
		if ((si4703_registers[STATUSRSSI] & (1 << STC)) == 0)
			break; //Tuning complete!
	}
}

/*************************************************************************
 Odczyt kanału
 Wczytuje nastawy aktualnego kanału z rejestru
 Parametry:	brak
 Zwraca: uint16_t kanał x 10 (liczba całkowita)
*************************************************************************/
//Reads the current channel from READCHAN
//Returns a number like 973 for 97.3MHz
uint16_t fm_getChannel10x(void) {
	si4703_readRegisters();
	uint16_t channel = si4703_registers[READCHAN] & 0x03FF; 			//Mask out everything but the lower 10 bits
	//Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
	channel += 875; 													//X=0.1*Chan+87.5/*10 ; kanal10x=98 + 875 = 973
	kanal10x=channel;
	return (channel);
}


/*************************************************************************
 Inicjalizacja si4703
 Włączenie I2C, właczenie radia, ustawienia w rejestrach dla Europy
 Parametry:	brak
*************************************************************************/
void si4703_init(void) {

	//	1. RESET I SDIO jako wyjścia
	DDR(SI4703_RESET_PORT) |= (1 << SI4703_RESET_PIN);
	DDR(SI4703_SDIO_PORT) |= (1 << SI4703_SDIO_PIN);

	//	2. Wybór sterowanie - 2 przewody trybem 1; s.6 SDIO LOW, SEN HI (na płytce)
	LO_SDIO;									// WYBÓR TRYBU I2C
	LO_RS;										// reset układu
	_delay_ms(1);
	HI_RS;										// WŁĄCZENIE UKŁADU RAZEM Z SDIO LOW (program), SEN HI (podciągnięte na płytce) -> WYBÓR I2C

	i2c_init(I2CBITRATE);						//inicjalizacja i2c
	si4703_readRegisters();
	//	3. procedura włączania
	si4703_registers[TEST1] |= (1<<XOSCEN);			// Ustaw oscylator, AN230 page 12, rev 0.9
	si4703_writeBuffor(TEST1,1);				// wyślij rejestr
	_delay_ms(500);
	si4703_readRegisters();					// aktualizacja danych rejestrów po stabilizacji oscylatora
	si4703_registers[RDSD] = 0x0000;			// reset danych RDS Si4703-C19 Errata Solution 2: Set RDSD = 0x0000
	si4703_writeBuffor(RDSD,1);				// wyślij rejestr
	//	4. radio włączone, konfiguracja
	si4703_readRegisters();					// aktualizacja danych
	si4703_registers[POWERCFG] |= (1<<DMUTE)|(1<<ENABLE);		// Wyłącz mute; włącz radio
	si4703_registers[SYSCONFIG1] |= (1<<RDS); 	//Enable RDS
	si4703_registers[SYSCONFIG1] |= (1<<DE); 	//50kHz Europe setup
	si4703_registers[SYSCONFIG2] |= (1<<SPACE0); 				//100kHz channel spacing for Europe
	si4703_registers[SYSCONFIG2] &= 0xFFF0; 	// Clear volume bits
	si4703_registers[SYSCONFIG2] |= 0x0003; 	// Mała głośność
	si4703_writeRegisters2_7(); 				//Update
	_delay_ms(110);								// powerup time p.12 AN230
	si4703_readRegisters();					// aktualizacja danych


}

/*************************************************************************um
 Odczyt rejestrów (si4703 wysyła od 0x0A)
 Parametry:	uint8_t SLA adres urządzenia
*************************************************************************/
void si4703_readRegisters(void) {
	i2c_start(SI4703_ADDR | I2C_READ);						// start transmicji	adres + 1
	for (uint8_t x = 0x0A;; x++) { 							//Read in these 32 bytes
		if (x == 0x10)	x = 0; 								//Loop back to zero
		if (x == 0x09) {									//ostatni bajt
			si4703_registers[x] = i2c_readAck() << 8;		// najpierw górny
			si4703_registers[x] |= i2c_readNak();			//ostatni bajt + nack
			break;
		}
		si4703_registers[x] = i2c_readAck() << 8;			// najpierw górny
		si4703_registers[x] |= i2c_readAck();				// teraz dolny
	}
}


/*************************************************************************
 Zapisz bufora do I2C (si4703 zapisuje automatycznie od 0x02)
 zapis rejestrów kontrolnych do 0x07
 Parametry:	brak
*************************************************************************/
void si4703_writeRegisters2_7(void) {
	i2c_start(SI4703_ADDR | I2C_WRITE);						// start transmicji	adres + 0
	for (reg_index=0x02; reg_index<0x08; reg_index++) {		// od rejestru 0x02 do 0x07
		i2c_write(si4703_registers[reg_index] >> 8);		// górny bajt
		i2c_write(si4703_registers[reg_index] & 0x00FF);	// dolny bajt
	}
	i2c_stop();
}


/*************************************************************************
 Zapisz bufora do rejestrów
 Zapisuje dane z bufora od wskzanego adresu o długości len
  Parametry:	uint8_t adress - adres startowy, uint8_t len - ilość rejestrów do zapisu
*************************************************************************/
void si4703_writeBuffor(uint8_t adress, uint8_t len) {
	i2c_start(SI4703_ADDR | I2C_WRITE);						// start transmicji	adres + 0
	i2c_write(adress);										// pod jaki adres
	while (len--) {
		i2c_write(si4703_registers[adress] >> 8);		// górny bajt
		i2c_write(si4703_registers[adress] & 0x00FF);	// dolny bajt
		adress++;
	}
	i2c_stop();
}


/*************************************************************************
 Czyta 1 rejestr
 Czyta rejestr spod adresu i zapisuje w buforze lokalnym
  Parametry:	uint8_t adress - adres rejestru
*************************************************************************/
void si4703_read1register(uint8_t adress) {
	i2c_start(SI4703_ADDR | I2C_READ);						// start transmicji	adres + 1
	i2c_write(adress);										// pod jaki adres
	si4703_registers[adress] = i2c_readAck() << 8;		// najpierw górny
	si4703_registers[adress] |= i2c_readNak();			//ostatni bajt + nack
	i2c_stop();
}

