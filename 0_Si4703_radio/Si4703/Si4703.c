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



//Inicjalizacja
void si4703_init(void) {

	//	1. RESET I SDIO jako wyjścia
	DDR(SI4703_RESET_PORT) |= (1 << SI4703_RESET_PIN);
	DDR(SI4703_SDIO_PORT) |= (1 << SI4703_SDIO_PIN);

	//	2. Wybór sterowanie - 2 przewody trybem 1; s.6 SDIO LOW, SEN HI (na płytce)
	LO_SDIO;									// WYBÓR TRYBU I2C
	LO_RS;										// reset układu
	_delay_ms(1);
	HI_RS;										// WŁĄCZENIE UKŁADU RAZEM Z SDIO LOW (program), SEN HI (podciągnięte na płytce) -> WYBÓR I2C
	si4703_pull();
	si4703_registers[OSCCTRL] = 0x8100;			//Enable the oscillator, from AN230 page 12, rev 0.9 (works)
}


/*//Read the entire register control set from 0x00 to 0x0F
uint8_t fm_readRegisters(void){
	int i = 0;
	int x;

	//Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
	uint8_t result = read_i2c_device(SI4703_READ, 32, isc_buffer);
	i2c_timerproc();

	//Remember, register 0x0A comes in first so we have to shuffle the array around a bit
	for(x = 0x0A ; ; x++) { //Read in these 32 bytes
		if(x == 0x10) x = 0; //Loop back to zero
		si4703_registers[x] = isc_buffer[i++] << 8;
		si4703_registers[x] |= isc_buffer[i++];
		if(x == 0x09) break; //We're done!
	}

	return result;
}*/

//The device starts reading at reg_index 0x0A, and has 32 bytes
void si4703_pull(void) {
	i2c_start(SI4703_ADDR | I2C_READ);
	for(int x = 0x0A ; ; x++) { 							//Read in these 32 bytes zacznij od [10]
		if(x == 0x10) x = 0; 								//Loop back to zero na [16] adresie
		si4703_registers[x] = i2c_readAck() << 8;
		si4703_registers[x] |= i2c_readAck();

		if(x == 0x08) break; 								//We're [almost] done!
	}
//todo sprawdzić na wyświetlaczu to dziwo
	si4703_registers[0x09] = i2c_readAck() << 8;
	si4703_registers[0x09] |= i2c_readNak();
}

