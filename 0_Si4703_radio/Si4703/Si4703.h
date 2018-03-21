/*
 * Si4703.h
 *
 *  Created on: 10 mar 2018
 *      Author: dago
 *      SENB or SEN—serial enable pin, active low, used only for 3-wire operation
 *		SDIO—serial data in/data out pin.
 *		SCLK—serial clock pin.
 * 		RSTB or RST—reset pin, active low
 */

#ifndef SI4703_H_
#define SI4703_H_


//----------------------------------------------------------------------------------------
//
//		Ustawienia sprzętowe połączeń sterownika z mikrokontrolerem
//
//----------------------------------------------------------------------------------------
// tu konfigurujemy port i piny do jakich podłączymy linie RESET, SDIO
#define SI4703_RESET_PORT	B
#define SI4703_RESET_PIN 	6
#define SI4703_SDIO_PORT  	B
#define SI4703_SDIO_PIN		5

// makrodefinicje operacji na sygnałach sterujących RESET I SDIO—serial data in/data out pin
#define HI_RS 	PORT(SI4703_RESET_PORT) |= (1<<SI4703_RESET_PIN)					// stan wysoki na linii RESET
#define LO_RS 	PORT(SI4703_RESET_PORT) &= ~(1<<SI4703_RESET_PIN)					// stan niski na linii RESET

#define HI_SDIO 	PORT(SI4703_SDIO_PORT) |= (1<<SI4703_SDIO_PIN)					// stan wysoki na SDIO
#define LO_SDIO 	PORT(SI4703_SDIO_PORT) &= ~(1<<SI4703_SDIO_PIN)					// stan niski na SDIO


// ********************* ustalić prawdę !!!!!!!!!!! **************************************
#define SI4703_ADDR 0x20
//#define SI4703_READ  0x21
//#define SI4703_WRITE 0x20

#define I2CBITRATE 400
//------------------------------------------------  koniec ustawień sprzętowych ---------


//definitions
extern uint8_t stereo;
enum DIRECTION {
	DOWN,
	UP,
};
//------------------------------------------------  rejestry SI4703 ---------------------

// Nazwy rejestrów
#define DEVICEID 0x00
#define CHIPID 0X01
#define POWERCFG 0x02
#define CHANNEL 0x03
#define SYSCONFIG1 0x04
#define SYSCONFIG2 0x05
#define SYSCONFIG3 0x06
#define TEST1 0x07
#define TEST2 0x08
#define BOOTCONFIG 0x09
#define STATUSRSSI 0x0A
#define READCHAN 0x0B
#define RDSA 0x0C
#define RDSB 0x0D
#define RDSC 0x0E
#define RDSD 0x0F

// Register 0x02 POWERCFG
#define DSMUTE 15
#define DMUTE 14
#define MONO 13
#define RDSM 11
#define SKMODE 10
#define SEEKUP 9
#define SEEK 8
#define DISABLE 6
#define ENABLE 0

// Register 0x03 CHANNEL
#define TUNE 15

// Register 0x04 SYSCONFIG1
#define RDSIEN 15
#define STCIEN 14
#define RDS 12
#define DE 11
#define AGCD 10
#define BLNDADJ1 7
#define BLNDADJ0 6
#define GPIO31 5
#define GPIO30 4
#define GPIO21 3
#define GPIO20 2
#define GPIO11 1
#define GPIO10 0

// Register 0x05 SYSCONFIG2
#define BAND1 7
#define BAND0 6
#define SPACE1 5
#define SPACE0 4
#define VOLUME3 3
#define VOLUME2 2
#define VOLUME1 1
#define VOLUME0 0

// Register 0x06 SYSCONFIG3
#define SMUTER1 15
#define SMUTER0 14
#define SMUTEA1 13
#define SMUTEA2 12
#define VOLEXT 8
#define SKSNR3 7
#define SKSNR2 6
#define SKSNR1 5
#define SKNSR0 4
#define SKCNT3 3
#define SKCNT2 2
#define SKCNT1 1
#define SKCNT0 0

// Register 0x07 TEST1
#define XOSCEN 15
#define AHIZEN 14

// Register 0x0A STATUSRSSI
#define RDSR 15
#define STC 14
#define SFBL 13
#define AFCRL 12
#define RDSS 11
#define BLERA1 10
#define BLERA0 9
#define ST 8
#define RSSI7 7
#define RSSI6 6
#define RSSI5 5
#define RSSI4 4
#define RSSI3 3
#define RSSI2 2
#define RSSI1 1
#define RSSI0 0

//Register 0x0B READCHAN
#define BLERB1 15
#define BLERB0 14
#define BLERC1 13
#define BLERC0 12
#define BLERD1 11
#define BLERD0 10
#define READCHAN9 9
#define READCHAN8 8
#define READCHAN7 7
#define READCHAN6 6
#define READCHAN5 5
#define READCHAN4 4
#define READCHAN3 3
#define READCHAN2 2
#define READCHAN1 1
#define READCHAN0 0





#define SI4703_FM_HIGH               1079
#define SI4703_FM_LOW                881
#define SI4703_VOL_HIGH              15
#define SI4703_VOL_LOW               0



// ---------------------------------------Makra upraszczające dostęp do portów ----------------
// *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// *** PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)


//------------------------------------------------  funkcje użytkowe ---------------------------

extern void si4703_init(void);
/*************************************************************************
 Odczyt rejestrów (si4703 wysyła od 0x0A)
 Parametry:	uint8_t SLA adres urządzenia
*************************************************************************/
extern void si4703_readRegisters(void);

/*************************************************************************
 Zapisz bufora do I2C (si4703 zapisuje automatycznie od 0x02)
 zapis rejestrów kontrolnych do 0x07
 Parametry:	brak
*************************************************************************/
extern void si4703_writeRegisters2_7(void);

// eksperyment!
void si4703_writeBuffor(uint8_t adress, uint8_t len);
void si4703_read1register(uint8_t adress);
uint8_t fm_setVolume(uint8_t volume);
uint16_t fm_seek(enum DIRECTION dir);
uint16_t fm_getChannel10x(void);
void fm_setChannel(uint16_t channel10x);


//functions
//void si4703_set_volume(uint8_t volume);
//void si4703_set_channel(int newChannel);
//uint16_t si4703_get_channel(void);
//uint8_t si4703_seek(uint8_t direction);

//void si4703_power_on(void);
//void si4703_pull(void);
//void si4703_push(void);
//void seek_TWI_devices();


#endif /* SI4703_H_ */
