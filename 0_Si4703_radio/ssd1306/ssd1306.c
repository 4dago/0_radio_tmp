
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "ssd1306.h"

#include "../I2C_TWI/i2c_master.h"

//***************************** bufor ekranu ***********************************
//******************************************************************************
    uint8_t ssd1306_buf[BUF_SIZE]={};

    //**************************************************************************

void TWI_write_buf(uint8_t SLA, uint8_t adr, uint16_t len, uint8_t *buf) {

	i2c_start(SLA);
	i2c_write(adr);
	while (len--)
		i2c_write(*buf++);
	i2c_stop();
}


//***************************** wysyłanie komend na i2C ************************
//******************************************************************************

void SSD1306_cmd(uint8_t cmd) {
	uint8_t control = 0x00;
	i2c_start( SSD1306_I2C_ADDRESS);
	i2c_write(control);
	i2c_write(cmd);
	i2c_stop();
}

//***************************** wysyłanie danych na i2C ************************
//******************************************************************************
void SSD1306_data(uint8_t dat) {

	// funkja wysyłania danych na spi
	uint8_t control = 0x40;
	i2c_start( SSD1306_I2C_ADDRESS);
	i2c_write(control);
	i2c_write(dat);
	i2c_stop();

}

//***************************** Czyszczenie ekranu *****************************
//******************************************************************************

void SSD1306_cls(void) {
	memset(ssd1306_buf, 0x00, (1024));
}

//***************************** Wyświetlanie ekranu ************************
//**************************************************************************
void SSD1306_display(void) {
// ************************ niepotrzebn w adresowaniu horizontal ************************

//                SSD1306_cmd( SSD1306_SETLOWCOLUMN       | 0x0 );			// kolumna min. =0 TRYB PAGE!!!
//                SSD1306_cmd( SSD1306_SETHIGHCOLUMN      | 0x0 );			// kolumna max. = 0 TRYB PAGE!!!
//                SSD1306_cmd( SSD1306_SETSTARTLINE       | 0x0 );			// linia nr. 0
// **************************************************************************************
	TWI_write_buf( SSD1306_I2C_ADDRESS, 0x40, 1024, ssd1306_buf);
}


//********** FUNKCJA ZAPISUJĄCA STAN PIXALA W BUFORZE **********************
//**************************************************************************
void SSD1306_setPixel(int x, int y, uint8_t bw) {
	if ((x < 0) || (x >= SSD1306_WIDTH) || (y < 0) || (y >= SSD1306_HEIGHT))
		return;

	if (bw)
		ssd1306_buf[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));		// zapal
	else
		ssd1306_buf[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));	// zgaś

}

//********** inicjacja  wyswietlacza I2C /odcinek 1a mówi o SPI ************
//**************************************************************************
void SSD1306_init(uint8_t vcc, uint8_t refresh) {

//	i2cSetBitrate(400);
	I2C_DIR |= (1 << SCL) | (1 << SDA);
	_delay_ms(50);

	SSD1306_cmd(SSD1306_DISPLAYOFF);
	SSD1306_cmd(SSD1306_SETDISPLAYCLOCKDIV);
	SSD1306_cmd(refresh);

	SSD1306_cmd(SSD1306_SETDISPLAYOFFSET);
	SSD1306_cmd(0x00);

	SSD1306_cmd(SSD1306_SETSTARTLINE | 0x0);
	SSD1306_cmd(SSD1306_CHARGEPUMP);

	if (vcc == SSD1306_EXTERNALVCC)
		SSD1306_cmd(0x10);
	else
		SSD1306_cmd(0x14);

	SSD1306_cmd(SSD1306_MEMORYMODE);						// adresowanie pamięci
	SSD1306_cmd(0x00);										// horizontal

	SSD1306_cmd(SSD1306_SEGREMAP | 0x1);
	SSD1306_cmd(SSD1306_COMSCANDEC);

	SSD1306_cmd(SSD1306_SETCONTRAST);

	if (vcc == SSD1306_EXTERNALVCC)
		SSD1306_cmd(0x9F);
	else
		SSD1306_cmd(0x01);

	SSD1306_cmd(SSD1306_SETPRECHARGE);

#if defined SSD1306_128_32

	SSD1306_cmd(SSD1306_SETMULTIPLEX);
	SSD1306_cmd(0x1F);
	SSD1306_cmd(SSD1306_SETCOMPINS);
	SSD1306_cmd(0x02);

#endif

#if defined SSD1306_128_64

	SSD1306_cmd(SSD1306_SETMULTIPLEX);
	SSD1306_cmd(0x3F);
	SSD1306_cmd(SSD1306_SETCOMPINS);
	SSD1306_cmd(0x12);

#endif
	SSD1306_cmd(SSD1306_SETVCOMDETECT);
	SSD1306_cmd(0x40);
	SSD1306_cmd(SSD1306_DISPLAYALLON_RESUME);
	SSD1306_cmd(SSD1306_NORMALDISPLAY);
	SSD1306_cmd(SSD1306_DISPLAYON);

	//*************** odświeżanie całego zakresu ekranu ******************* DZIAŁA !
	// https://www.avrfreaks.net/forum/ssd1306-lcd-initialization-commands

	SSD1306_cmd(SSD1306_COLUMNADDR); 					// 0x21 COMMAND
	SSD1306_cmd(0); 									// Column start address
	SSD1306_cmd(SSD1306_WIDTH-1); 						// Column end address

	SSD1306_cmd(SSD1306_PAGEADDR); 					// 0x22 COMMAND
	SSD1306_cmd(0); 									// Start Page address
	SSD1306_cmd((SSD1306_HEIGHT/8)-1);					// End Page address



}

//********** Odświeżanie stronami ******************************************
//**************************************************************************
void SSD1306_refreshPages(uint8_t page_start, uint8_t page_end,
		uint8_t col_start, uint8_t col_end) {
// ustaw zakres odświeżania
	uint8_t page_cnt, col_cnt;									//liczba stron,liczba kolumn
	uint8_t * ram_buf_start;

	SSD1306_cmd(SSD1306_COLUMNADDR); 							// 0x21 COMMAND
	SSD1306_cmd(col_start); 									// Column start address
	SSD1306_cmd(col_end); 										// Column end address

	SSD1306_cmd(SSD1306_PAGEADDR); 							// 0x22 COMMAND
	SSD1306_cmd(page_start); 									// Start Page address
	SSD1306_cmd(page_end);										// End Page address

	i2c_start( SSD1306_I2C_ADDRESS);
	i2c_write(0x40);
	for (page_cnt = page_start; page_cnt < (page_end + 1); page_cnt++) {
//		SSD1306_cmd(SSD1306_SETLOWCOLUMN | (col_start & 0x0F)); 		// starszy oktet kolumny
//		SSD1306_cmd(SSD1306_SETHIGHCOLUMN | col_start >> 4);			// młodszy oktet kolumny
//		SSD1306_cmd(0xB0 + page_cnt);									// aktualny page

		ram_buf_start = &ssd1306_buf[(page_cnt * 128) + col_start];	// bajt startowy

		for (col_cnt = col_start; col_cnt < col_end + 1; col_cnt++) {
			i2c_write(*ram_buf_start++);
		}
		i2c_stop();
	}
	// Powtót do odświeżania całego zakresu ekranu
	SSD1306_cmd(SSD1306_COLUMNADDR); 				// 0x21 COMMAND
	SSD1306_cmd(0); 								// Column start address
	SSD1306_cmd(SSD1306_WIDTH - 1); 				// Column end address

	SSD1306_cmd(SSD1306_PAGEADDR); 				// 0x22 COMMAND
	SSD1306_cmd(0); 								// Start Page address
	SSD1306_cmd((SSD1306_HEIGHT / 8) - 1);			// End Page address
}

