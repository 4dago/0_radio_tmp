/*
 * Si4703.c
 *
 *  Created on: 10 mar 2018
 *      Author: dago
 */
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

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
int8_t glosnosc;
uint8_t godziny;
uint8_t minuty;
uint8_t offsetutc;

// allowed chars in RDS
#define FIRST_ALLOWED_CHAR (0x20)
#define LAST_ALLOWED_CHAR (0x7f)

// rds buffering
uint8_t RDS_PSready = 0;
uint8_t RDS_RTready = 0;
uint8_t RDS_CTready = 0;
char rdsdata[9];
char radiotext[65];
uint8_t rdschanged = 0;
uint8_t fakerds = 1;
uint8_t rssi = 0x19;
uint8_t sksnr = 0x4;
uint8_t skcnt = 0x8;

// RDS specific stuff
#define RDS_PS (0)
#define RDS_RT (2)
#define RDS_CT (4)

#define RDS_GROUP_TYPE_0A (0)
#define RDS_GROUP_TYPE_0B (1)
#define RDS_GROUP_TYPE_2A (4)
#define RDS_GROUP_TYPE_2B (5)
#define RDS_GROUP_TYPE_4A (8)


//-------------------------------------------------------------------------
//					Deklaracje funkcji lokalnych
//-------------------------------------------------------------------------
void clearStringBuff(char * buf, uint8_t textsize);
void clearRDSBuff(void);
//-------------------------------------------------------------------------


//todo Czy tylko ten zakres? brak 0x0D (cr), 0x0A - line feed Code 0x0B: end of headline 0x1F: soft hyphen itd
static inline void considerrdschar(char * buf, uint8_t place, char ch) {
	if (ch < FIRST_ALLOWED_CHAR || ch > LAST_ALLOWED_CHAR)
		return;

	buf[place] = ch;
}


void clearRDSBuff(void) {
//	uint16_t channel = (si4703_registers[READCHAN] & 0x03FF) + 875;				//zrobić inaczej
	clearStringBuff(rdsdata, 8);
	clearStringBuff(radiotext, 64);
	godziny = 0;
	minuty = 0;
	offsetutc = 0;
//	str_putfreq(rdsdata, channel, 0);											//zrobic inaczej
	rdschanged = 1;
	fakerds = 1;
}


void clearStringBuff(char * buf, uint8_t textsize) {
	memset(buf, ' ', textsize);
	buf[textsize] = 0;
}


uint8_t str_putfreq(char * str, uint16_t freq, uint8_t start) {
	start = str_putrawfreq(str, freq, start);
	str[start++] = 'M';
	str[start++] = 'H';
	str[start++] = 'z';
	return start;
}


uint8_t str_putrawfreq(char * str, uint16_t freq, uint8_t start) {
	uint8_t fract = freq % 10;
	uint8_t whole = freq / 10;

	start = str_putuint8(str, whole, start);
	str[start++] = '.';
	start = str_putuint8(str, fract, start);
	return start;
}



// draws a uint8 onto a string and returns position of last char
uint8_t str_putuint8(char * str, uint8_t val, uint8_t start) {

	char buf[3];
	int8_t ptr;
	for(ptr=0;ptr<3;++ptr) {
		buf[ptr] = (val % 10);
		val /= 10;
	}
	for(ptr=2;ptr>0;--ptr) {
		if (buf[ptr] != 0) break;

	}
	for(;ptr>=0;--ptr) {
		str[start] = '0'+buf[ptr];
		start++;
	}
	return start;
}



//todo słabe, tylko 0A, 2A, zamieszanie ze zmiennymi, powielanie buforów, flaga A/B; do poprawy
uint8_t fm_readRDS(char* ps, char* rt) {
	si4703_readRegisters();
	if(si4703_registers[STATUSRSSI] & (1<<RDSR)) {						// Nowe dane w rejestrze
		if (fakerds) {													// czy złe? / pierwsze uwłaczenie
			memset(rdsdata, ' ', 8);									// Czyść string
			rdschanged = 1;
		}

		fakerds = 0;													// zerowanie flagi

		//const uint16_t a = si4703_registers[RDSA];
		const uint16_t b = si4703_registers[RDSB];						// Najważniejszy blok logiki
		const uint16_t c = si4703_registers[RDSC];
		const uint16_t d = si4703_registers[RDSD];

		const uint8_t groupid = (b & 0xF000) >> 12;					// Jaka grupa? 16 wersji A i B =32 możliwośći
		//uint8_t version = b & 0x10;
		switch(groupid) {
			case RDS_PS: {												// Grupa 0A lub 0B, dane w bloku D, index to C0-C1 (blok B)
				const uint8_t index = (b & 0x3)*2;						// max.8znaków +0, inedx: 0,2,4,6
				// Jeśli index 0x0 nowy tekst, powinno być czyszczenie bufora
//				if (0==index) clearRDSBuff();							// jakoś inaczej
				char Dh = (d & 0xFF00) >> 8;							// Starszy
				char Dl = d;											// Młodszy

				considerrdschar(rdsdata, index, Dh);					// Zapis do bufora
				considerrdschar(rdsdata, index +1, Dl);

				rdschanged = 1;
			};
			break;
			case RDS_RT: {												// Radio tekst 64 znaki+0
				const uint8_t index = (b & 0xF)*4;						// 0, 4,8...64 (wersja 2A bloku RT)
				// Jeśli index 0x0 nowy tekst, powinno być czyszczenie bufora
				char Ch = (c & 0xFF00) >> 8;
				char Cl = c;
				char Dh = (d & 0xFF00) >> 8;
				char Dl = d;
				// UWAGA Text A/B flag: If the receiver detects a change in the flag (from binary "0" to binary "1" or vice-versa), then the whole
				// RadioText display should be cleared and the newly received RadioText message segments should be
				// written into the display;
				considerrdschar(radiotext, index, Ch);
				considerrdschar(radiotext, index +1, Cl);
				considerrdschar(radiotext, index +2, Dh);
				considerrdschar(radiotext, index +3, Dl);

				rdschanged = 1;
			};
			break;
			case RDS_CT: {												// godzina, daty nie dekoduję
				minuty = (d & 0x0FC0) >> 6;								//maskowanie minut
				godziny = ((d & 0xF000) >> 12) | ((c & 0x0001) << 5);
				offsetutc = (d & 0x001F);
							rdschanged = 1;
						};
						break;
		}
	}

	const uint8_t change = rdschanged;
	if (change) {
		strcpy(ps, rdsdata);
		strcpy(rt, radiotext);
	}
	rdschanged = 0;
	return (change) ? ((fakerds) ? (RDS_FAKE) : (RDS_AVAILABLE)) : (RDS_NO);
}


/*************************************************************************
 Ustawia głośniść
 Ustawia głośność 0 - 15 (prosty wariant ustawiania)
  Parametry:	uint8_t volume - głośność
  todo: udoskonalić, uwzględnić wersje z VOLEXT 06h[8] bit, sprawdzić inne rejestry
*************************************************************************/
void fm_setVolume(int8_t volume) {
  si4703_readRegisters();		//Read the current register set
  if(volume < 0) volume = 0;
  if (volume > 15) volume = 15;
  glosnosc = volume;						//zapamiętaj głośność globalnie
  si4703_registers[SYSCONFIG2] &= 0xFFF0; 	//Clear volume bits
  si4703_registers[SYSCONFIG2] |= volume; 	//Set new volume
//  si4703_writeBuffor(SYSCONFIG2,1);  		//Update
si4703_writeRegisters2_7(); 				//Update

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
uint16_t fm_seek(enum DIRECTION dir, uint8_t rssi, uint8_t snr, uint8_t cnt) {
	si4703_readRegisters();
	si4703_registers[POWERCFG] &= ~(1 << SKMODE); 			//enable wrapping of frequencies
	if (dir == DOWN) {
//		printf("up\n");
		si4703_registers[POWERCFG] &= ~(1<<SEEKUP);
	} else {
//		printf("down\n");
		si4703_registers[POWERCFG] |= (1<<SEEKUP);
	}
	si4703_registers[SYSCONFIG2] &= 0x00FF;					// czyść rssi
	si4703_registers[SYSCONFIG2] |= rssi<<8;				// ustaw rssi

	si4703_registers[SYSCONFIG3] &= 0x00FF;					// czyść sksnr i skcnt
	si4703_registers[SYSCONFIG3] |= (snr<<4)|(cnt);				// ustaw rssi

	si4703_registers[POWERCFG] |= (1<<SEEK); 				//start seeking
//	si4703_writeBuffor(POWERCFG,1);

	si4703_writeRegisters2_7(); 							//Update

	_delay_ms(60);											// nota

	while (!(si4703_registers[STATUSRSSI] & (1<<STC))) {		//coś znalazł
		si4703_readRegisters();
	} 															//seek complete!
//	si4703_readRegisters();									// aktualizacja
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
	//9.8 / 0.1 = 49
/* BEZ SENSU DLA EUROPY
	uint16_t newChannel = channel10x * 10; 						//973 * 10 = 9730
	newChannel -= 8750; 										//9730 - 8750 = 980
	newChannel /= 10; 											//980 / 10 = 98
*/
	uint16_t newChannel = channel10x - 875; 						//973 -875 = 98

	//These steps come from AN230 page 20 rev 0.5
	si4703_readRegisters();
	si4703_registers[CHANNEL] &= 0xFE00; 						//Clear out the channel bits
	si4703_registers[CHANNEL] |= newChannel; 					//Mask in the new channel
	si4703_registers[CHANNEL] |= (1 << TUNE); 					//Set the TUNE bit to start

	si4703_writeRegisters2_7(); 				//Update

//	si4703_writeBuffor(CHANNEL,1);
	_delay_ms(60); 												//Wait 60ms - nota
	//Poll to see if STC is set
	while (1) {
		si4703_readRegisters();
		if ((si4703_registers[STATUSRSSI] & (1 << STC)) != 0)
			break; //Tuning complete!
	}
	si4703_registers[CHANNEL] &= ~(1 << TUNE); //Clear the tune after a tune has completed
//	si4703_writeBuffor(CHANNEL,1);

	si4703_writeRegisters2_7(); 				//Update

	//Wait for the si4703 to clear the STC as well
	while (1) {
		si4703_readRegisters();
		if ((si4703_registers[STATUSRSSI] & (1 << STC)) == 0)
			break; //Tuning complete!
	}
}


/*


 Procedura ISR INT0 odpowiedzialna za obsługę wiadomosci RDS. Komplet danych RDS (4, 16-bit. rejestry układu Si4703)
 sygnalizowany jest poprzez sciągnięcie wyprowadzenia GPIO2 układu Si4703 (wejscie INT0 mikrokontrolera) do „0” przez
 czas 5ms. Poszczególne rejestry RDS zawierają następujące dane:
 RDSA - pomijamy, gdyż zawiera nieinteresujące nas dane (Program Inentyfi cation Code tzw.PI)
 RDSB - bity 15...11 okreslają typ wiadomosci (Group Type +Version) zas bity 1...0 dla typu 0A/0B lub 3...0 dla typu
 2A/2B określają indeks przesłanego segmentu danych (segment danych to 2 kolejne znaki wiadomosci dla typu 0A/0B czy 2B
 lub 4 znaki dla typu 2A). W przypadku wiadomości typu 4A (CT) najmłodsze 2 bity tego rejestru stanowią najstarsze bity
 daty zapisanej w konwencji MJD.
 RDSC - dla typu 0A/0B i 2B pomijamy, dla typu 2A zawiera kody ASCII 2 znaków bieżącego segmentu danych (wiadomosci typu
 PS lub RT), zaś dla typu 4A zawiera datę w zapisie MJD oraz najstarszy bit godziny (bit0 tegoż rejestru).
 RDSD - dla typu 0A/0B i 2A/2B zawiera kody ASCII 2 znaków bieżącego segmentu danych (wiadomosci typu PS lub RT) zas dla
 typu 4A zawiera godzinę (bity 15...12), minuty (bity 11...6) oraz offset lokalnego czasu (bity 5...0).
void fm_readRDS(char* ps, char* rt) {
	static uint8_t PScharIndex; //Indeks odebranego znaku wiadomosci PS (Program Service)
	static uint8_t RTcharIndex; //Indeks odebranego znaku wiadomosci RT (Radio Text)
	static char PStext[9]; //Deklaracja lokalnej tablicy przechowującej nazwę stacji (Program Service): max. 8 znaków + terminator
	static char RTtext[65]; //Deklaracja lokalnej tablicy przechowującej tekst (Radio Text): max. 64 znaki + terminator
	register uint8_t groupType; //Typ odebranej wiadomosci RDS umieszczony w starszym bajcie rej. RDSB
	register uint8_t receivedCharIndex; //Index bieżącego znaku obliczany na podstawie przesłanego indeksu segmentu danych PS/RT
	register uint8_t A_B; //Wskaźnik zmiany wiadomosci typu RT (niezbędny, gdyż ta sama wiadomosc może byc powtarzana wielokrotnie)
	static uint8_t RTchanged; //Znacznik zmiany tresci wiadomosci typu RT
	register uint8_t idx, messageReady = 0; //Zmienne pomocnicze
//Odczyt bieżących wartosci wyłącznie rejestrów 0x0A...0x0F układu Si4703 (rejestry o adresach 0x0C...0x0F to rejestry RDSA ...RDSD)
	si4703_readRegisters();
	if (si4703_registers[STATUSRSSI] & (1 << RDSR)) {
		groupType = (si4703_registers[RDSB] >> 11); //Typ przechwyconej wiadomosci RDS
		switch (groupType) {
		case RDS_GROUP_TYPE_0A: //Basic Tuning and Switching Information only - maks. 8 znaków
		case RDS_GROUP_TYPE_0B: //Basic Tuning and Switching Information only - maks. 8 znaków
//Na podstawie przesłanego indeksu segmentu danych obliczamy bieżący indeks znaku (segment*2, gdyż odbieramy po 2 znaki)
			receivedCharIndex = ((si4703_registers[RDSB] & 0x03) << 1);		// index *2
			//Sprawdzamy czy odebrany segment znaków ma oczekiwaną wartość indeksu co oznacza, że nie pominięto żadnej ramki
//			if (PScharIndex == receivedCharIndex) {
				idx = (si4703_registers[RDSD] >> 8); //Pierwszy, odebrany znak
				if (idx < ' ' || idx > '}')
					idx = ' '; //Poprawienie ewentualnych nieodpowiednich kodów znaków
				PStext[PScharIndex] = idx;
				idx = si4703_registers[RDSD]; //Drugi, odebrany znak
				if (idx < ' ' || idx > '}')
					idx = ' '; //Poprawienie ewentualnych nieodpowiednich kodów znaków
				PStext[PScharIndex + 1] = idx;
				PScharIndex += 2; //Zwiększamy bieżący index o 2 znaki
				if (PScharIndex == 8) //Odebrano kompletną wiadomosc typu Program Service (nazwa stacji)
						{
					PStext[8] = '\0'; //Terminator na końcu stringa
					strcpy(ps, PStext); //Kopiujemy otrzymaną wiadomosc do tablicy dostępnej dla innych modułów
					RDS_PSready = 1; //Ustawiamy fl agę otrzymania nowej wiadomosci RDS typu PS
					PScharIndex = 0; //Zerujemy index znaku
				}
//			} else
//				PScharIndex = 0; //Zerujemy index znaku, gdyż zgubiono częsc wiadomosci typu PS
			break;
		case RDS_GROUP_TYPE_2A: //Radio Text only - maks. 64 znaki
//Na podstawie przesłanego indexu segmentu danych obliczamy bieżący index znaku (segment*4, gdyż odbieramy po 4 znaki)
			receivedCharIndex = ((si4703_registers[RDSB] & 0x0F) << 2);
//Sprawdzamy czy odebrany segment znaków posiada oczekiwaną wartosc indeksu co oznacza, że nie pominięto żadnej ramki
			if (RTcharIndex == receivedCharIndex) {
				RTtext[RTcharIndex] = (si4703_registers[RDSC] >> 8); //Pierwszy, odebrany znak
				RTtext[RTcharIndex + 1] = si4703_registers[RDSC]; //Drugi, odebrany znak
				RTtext[RTcharIndex + 2] = (si4703_registers[RDSD] >> 8); //Trzeci, odebrany znak
				RTtext[RTcharIndex + 3] = si4703_registers[RDSD]; //Czwarty, odebrany znak
				//Korekta odebranych znaków: LF=’ ‚, CR=’\0’ (znacznik końca stringa)
				for (idx = 0; idx < 4; idx++) {
					if (RTtext[RTcharIndex + idx] == 0x0A)
						RTtext[RTcharIndex + idx] = ' '; //LF
					if (RTtext[RTcharIndex + idx] == 0x0D) {
						messageReady = 1;
						break;
					}
				}
				if (!messageReady) {
					RTcharIndex += 4;
					idx = 0;
				} //Zwiększamy bieżący index o 4 znaki
				  //Jesli nawet w przesłanej wiadomosci nie wystąpił znacznik końca stringa (CR) ale odebrano 64 znaki to kończymy
				  //odbieranie bieżącej wiadomosci tekstowej ustawiając odpowiednią fl agę i kopiując wiadomosc do zmiennej globalnej
				  //pod warunkiem, iż tresc odebranej wiadomosci jest inna niż ostatnio przesłana - o tym „mówi” wskaźnik A-B
				if (messageReady || RTcharIndex == 64) //Odebrano kompletną wiadomosc typu Radio Text (informacje tekstowe)
						{
					//Sprawdzamy czy odebrana własnie wiadomosc typu RT nie jest kopią poprzednio przesłanej - bit ten zmienia się
					//na wartosc przeciwną za każdym razem, gdy przesyłana jest nowa tresc. Dla kopii wiadomosci pozostaje bez zmian
					A_B = (si4703_registers[RDSB] >> 4) & 0x01;
					if (RTchanged != A_B) {
						RTtext[RTcharIndex + idx] = '\0'; //Terminator na końcu stringa
						strcpy(rt, RTtext); //Kopiujemy otrzymany tekst do tablicy dostępnej dla innych modułów
						RDS_RTready = 1; //Ustawiamy fl agę otrzymania nowej wiadomosci RDS typu PS
						RTchanged = A_B; //Aktualizujemy
					}
					RTcharIndex = 0; //Zerujemy index znaku
				}
			} else
				RTcharIndex = 0; //Zerujemy index znaku, gdyż zgubiono częsc wiadomosci typu RT
			break;
		case RDS_GROUP_TYPE_2B: //Radio Text only - maks. 32 znaki
//Na podstawie przesłanego indexu segmentu danych obliczamy bieżący index znaku (segment*2, gdyż odbieramy po 2 znaki)
			receivedCharIndex = ((si4703_registers[RDSB] & 0x0F) << 1);
//Sprawdzamy czy odebrany segment znaków posiada oczekiwaną wartosc indeksu co oznacza, że nie pominięto żadnej ramki
			if (RTcharIndex == receivedCharIndex) {
				RTtext[RTcharIndex] = (si4703_registers[RDSD] >> 8); //Pierwszy, odebrany znak
				RTtext[RTcharIndex + 1] = si4703_registers[RDSD]; //Drugi, odebrany znak
				//Korekta odebranych znaków: LF=’ ‚, CR=’\0’ (znacznik końca stringa)
				for (idx = 0; idx < 2; idx++) {
					if (RTtext[RTcharIndex + idx] == 0x0A)
						RTtext[RTcharIndex + idx] = ' '; //LF
					if (RTtext[RTcharIndex + idx] == 0x0D) {
						messageReady = 1;
						break;
					}
				}
				if (!messageReady) {
					RTcharIndex += 2;
					idx = 0;
				} //Zwiększamy bieżący index o 2 znaki
				  //Jesli nawet w przesłanej wiadomosci nie wystąpił znacznik końca stringa (CR) ale odebrano 32 znaki to kończymy
				  //odbieranie bieżącej wiadomosci tekstowej ustawiając odpowiednią fl agę i kopiując wiadomosc do zmiennej globalnej
				  //pod warunkiem, iż tresc odebranej wiadomosci jest inna niż ostatnio przesłana - o tym „mówi” wskaźnik A-B
				if (messageReady || RTcharIndex == 32) //Odebrano kompletną wiadomosc typu Radio Text (informacje tekstowe)
						{
					//Sprawdzamy czy odebrana własnie wiadomosc typu RT nie jest kopią poprzednio przesłanej - bit ten zmienia się
					//na wartosc przeciwną za każdym razem, gdy przesyłana jest nowa tresc. Dla kopii wiadomosci pozostaje bez zmian
					A_B = (si4703_registers[RDSB] >> 4) & 0x01;
					if (RTchanged != A_B) {
						RTtext[RTcharIndex + idx] = '\0'; //Terminator na końcu stringa
						strcpy(rt, RTtext); //Kopiujemy otrzymany tekst do tablicy dostępnej dla innych modułów
						RDS_RTready = 1; //Ustawiamy fl agę otrzymania nowej wiadomosci RDS typu PS
						RTchanged = A_B; //Aktualizujemy
					}
					RTcharIndex = 0; //Zerujemy index znaku
				}
			} else
				RTcharIndex = 0; //Zerujemy index znaku, gdyż zgubiono częsc wiadomosci typu RT
			break;
		case RDS_GROUP_TYPE_4A: //Clock Time and Date only
			minuty = (si4703_registers[RDSD] >> 6) & 0b111111;
			godziny = (si4703_registers[RDSD] >> 12);
			godziny |= ((si4703_registers[RDSC] & 0x01) << 4);
//Korygujemy obliczoną wartosc godzin o offset czasu lokalnego ( RDSD.5=0 -> offset+,RDSD.5=1 -> offset- )
//Offset podany jest jako wielokrotnosc wartosci pół godziny
			if (si4703_registers[RDSD] & 0b100000)
				godziny -= ((si4703_registers[RDSD] & 0b11111) >> 1);
			else
				godziny += ((si4703_registers[RDSD] & 0b11111) >> 1);
//Ustawiamy flagę otrzymania nowej wiadomosci RDS typu CT tylko w przypadku poprawnych danych
			if (godziny < 24 && minuty < 60)
				RDS_CTready = 1;
			break;
		}
	}
}

*/


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

//	i2c_init(I2CBITRATE);						//inicjalizacja i2c
	si4703_readRegisters();
	//	3. procedura włączania
	si4703_registers[TEST1] |= (1<<XOSCEN);			// Ustaw oscylator, AN230 page 12, rev 0.9
//	si4703_writeBuffor(TEST1,1);				// wyślij rejestr

	si4703_writeRegisters2_7(); 				//Update

	_delay_ms(500);
	si4703_readRegisters();					// aktualizacja danych rejestrów po stabilizacji oscylatora
	si4703_registers[RDSD] = 0x0000;			// reset danych RDS Si4703-C19 Errata Solution 2: Set RDSD = 0x0000
//	si4703_writeBuffor(RDSD,1);				// wyślij rejestr

	si4703_writeRegisters2_7(); 				//Update

	//	4. radio włączone, konfiguracja
	si4703_readRegisters();					// aktualizacja danych
	si4703_registers[POWERCFG] |= (1<<DMUTE)|(1<<ENABLE);		// Wyłącz mute; włącz radio
	si4703_registers[SYSCONFIG1] |= (1<<RDS); 	//Enable RDS (wersja standard)
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

/*	NIE DZIAŁA ZAPIS SELEKTYWNY REJESTRÓW
 *
************************************************************************
 Zapisz bufora do rejestrów
 Zapisuje dane z bufora od wskzanego adresu o długości len
  Parametry:	uint8_t adress - adres startowy, uint8_t len - ilość rejestrów do zapisu
************************************************************************
void si4703_writeBuffor(uint8_t adress, uint8_t len) {
	i2c_start(SI4703_ADDR | I2C_WRITE);						// start transmicji	adres + 0
	i2c_write(adress);										// pod jaki adres
	while (len--) {
		i2c_write(si4703_registers[adress] >> 8);		// górny bajt
		i2c_write(si4703_registers[adress] & 0x00FF);	// dolny bajt
		adress++;
	}
	i2c_stop();
}*/



/*  NIE DZIAŁA CZYTANIE SELEKTYWNE REJESTRÓW

************************************************************************
 Czyta 1 rejestr
 Czyta rejestr spod adresu i zapisuje w buforze lokalnym
  Parametry:	uint8_t adress - adres rejestru
************************************************************************
void si4703_read1register(uint8_t adress) {
	i2c_start(SI4703_ADDR | I2C_READ);						// start transmicji	adres + 1
	i2c_write(adress);										// pod jaki adres
	si4703_registers[adress] = i2c_readAck() << 8;		// najpierw górny
	si4703_registers[adress] |= i2c_readNak();			//ostatni bajt + nack
	i2c_stop();
}

*/
