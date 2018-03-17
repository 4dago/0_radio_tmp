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
#define SI4703_READ  0x21
#define SI4703_WRITE 0x20

#define I2CBITRATE 400
//------------------------------------------------  koniec ustawień sprzętowych ---------


//definitions
enum DIRECTION {
	DOWN,
	UP,
};
//------------------------------------------------  rejestry SI4703 ---------------------
// Nazwy rejestrów
#define DEVICEID    0x00
#define CHIPID      0x01
#define POWERCFG    0x02
#define CHANNEL     0x03
#define SYSCONFIG1  0x04
#define SYSCONFIG2  0x05
#define OSCCTRL     0x07
#define STATUSRSSI  0x0A
#define READCHAN    0x0B
#define RDSA        0x0C
#define RDSB        0x0D
#define RDSC        0x0E
#define RDSD        0x0F

// //Register 0x01 - CHIPID


//Register 0x02 - POWERCFG
#define SMUTE       15
#define DMUTE       14
#define SKMODE      10
#define SEEKUP      9
#define SEEK        8

//Register 0x03 - CHANNEL
#define TUNE        15

//Register 0x04 - SYSCONFIG1
#define RDS         12
#define DE          11

//Register 0x05 - SYSCONFIG2
#define SPACE1      5
#define SPACE0      4

//Register 0x0A - STATUSRSSI
#define RDSR        15
#define STC         14
#define SFBL        13
#define AFCRL       12
#define RDSS        11
#define STEREO      8

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
extern void si4703_readRegisters(void);

//functions
void si4703_set_volume(uint8_t volume);
void si4703_set_channel(int newChannel);
uint16_t si4703_get_channel(void);
uint8_t si4703_seek(uint8_t direction);

void si4703_power_on(void);
void si4703_pull(void);
void si4703_push(void);
//void seek_TWI_devices();


#endif /* SI4703_H_ */
