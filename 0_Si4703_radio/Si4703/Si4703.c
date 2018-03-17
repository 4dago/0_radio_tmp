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


uint16_t si4703_registers[16]; 					//There are 16 registers, each 16 bits large
uint8_t isc_buffer[32]; 						// the size of registers time 2 (since registers are 2 bytes each)



// Inicjalizacja
void si4703_init(void) {

	//	1. RESET I SDIO jako wyjścia
	DDR(SI4703_RESET_PORT) |= (1 << SI4703_RESET_PIN);
	DDR(SI4703_SDIO_PORT) |= (1 << SI4703_SDIO_PIN);

	//	2. Wybór sterowanie - 2 przewody trybem 1; s.6 SDIO LOW, SEN HI (na płytce)
	LO_SDIO;									// WYBÓR TRYBU I2C
	LO_RS;										// reset układu
	_delay_ms(1);
	HI_RS;										// WŁĄCZENIE UKŁADU RAZEM Z SDIO LOW (program), SEN HI (podciągnięte na płytce) -> WYBÓR I2C
	si4703_readRegisters();
	si4703_registers[OSCCTRL] |= 0x8000;			// Ustaw oscylator z maskowaniem, AN230 page 12, rev 0.9
}


// Odczyt rejestrów (chip wysyła od 0x0A)
void si4703_readRegisters(void){
	for(uint8_t x = 0x0A ; ; x++) { 			//Read in these 32 bytes
			if(x == 0x10) x = 0; 				//Loop back to zero
			si4703_registers[x] = i2c_readAck() << 8;
			si4703_registers[x] |= i2c_readAck();
			if(x == 0x08) break; 				//przedostani rejestr
		}
	si4703_registers[0x09] = i2c_readAck() << 8;
	si4703_registers[0x09] |= i2c_readNak();	//ostatni bajt + nack
}

/*************************************************************************
 Zapisz bufor do urządzenia I2C

Parametry:	uint8_t SLA adres urządzenia
			uint8_t adr adres startowy w pamięci
			uint8_t len ilość bajtów do zapisu
			uint16_t *buf bufor z danymi 16bit
*************************************************************************/
void i2c_write_buf16b(uint8_t SLA, uint8_t adr, uint8_t len, uint16_t *buf) {

	if (!i2c_start(SLA)) {
		i2c_write(adr);
		while (len--)
			i2c_write(*buf++);
		i2c_stop();
	}




	i2c_start_wait(SI4703_ADDR | I2C_WRITE);
	for (reg_index=0x00; reg_index<0x06; reg_index++) {
		i2c_write(si4703_data_registers[reg_index+2] >> 8);
		i2c_write(si4703_data_registers[reg_index+2] & 0x00FF);
	}

i2c_stop()

}



void i2c_read_buf(uint8_t SLA, uint8_t adr, uint8_t len, uint16_t *buf) {

	TWI_start();
	TWI_write(SLA);
	TWI_write(adr);
	TWI_start();
	TWI_write(SLA + 1);
	while (len--) *buf++ = TWI_read( len ? ACK : NACK );
	TWI_stop();
}

