

    #include <avr/io.h>
    #include <avr/pgmspace.h>
    #include <util/delay.h>
    #include <string.h>
    #include <stdlib.h>
    #include "ssd1306.h"


    uint8_t ssd1306_buf[1024]={                     // bufor


    };

    void i2cSetBitrate(uint16_t bitrateKHz) {
            uint8_t bitrate_div;

            bitrate_div = ((F_CPU/1000UL)/bitrateKHz);
            if(bitrate_div >= 16)
                    bitrate_div = (bitrate_div-16)/2;

            TWBR = bitrate_div;
    }

    void TWI_start(void) {
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
            while (!(TWCR&(1<<TWINT)));
    }

    void TWI_stop(void) {
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
            while ( !(TWCR&(1<<TWSTO)) );
    }

    void TWI_write(uint8_t bajt) {
            TWDR = bajt;
            TWCR = (1<<TWINT)|(1<<TWEN);
            while ( !(TWCR&(1<<TWINT)));
    }

    uint8_t TWI_read(uint8_t ack) {
            TWCR = (1<<TWINT)|(ack<<TWEA)|(1<<TWEN);

            while ( !(TWCR & (1<<TWINT)) );

            return TWDR;
    }


    void TWI_write_buf( uint8_t SLA, uint8_t adr, uint16_t len, uint8_t *buf) {

            TWI_start();
            TWI_write(SLA);
            TWI_write(adr);
            while (len--) TWI_write(*buf++);
            TWI_stop();
    }


    //***************************** wysyłanie komend na i2C

    void SSD1306_cmd(uint8_t cmd){


    // funkcja wysyłania rozkazu po i2c
                                    uint8_t control = 0x00;
                        TWI_start();
                        TWI_write( SSD1306_I2C_ADDRESS );
                        TWI_write( control );
                        TWI_write( cmd );
                        TWI_stop();

    }
    //*******************************************************************

    //**********************************wysyłanie danych
    void SSD1306_data(uint8_t dat){ // procedura tysłania danych na spi


    // funkja wysyłania danych na spi
                        uint8_t control = 0x40;
                        TWI_start();
                        TWI_write( SSD1306_I2C_ADDRESS );
                        TWI_write( control );
                        TWI_write( dat );
                        TWI_stop();

    }

    //*********************************************************************

    void SSD1306_cls(void){
            memset(ssd1306_buf,0x00,(1024));
    }

    // *****************************przeniesienie bufora na lcd
    void SSD1306_display( void )
        {
                SSD1306_cmd( SSD1306_SETLOWCOLUMN       | 0x0 );
                SSD1306_cmd( SSD1306_SETHIGHCOLUMN      | 0x0 );
                SSD1306_cmd( SSD1306_SETSTARTLINE       | 0x0 );


                //Funkcje od TWI są takie same jak w BB. Jedyna modyfikacja:
                   // void TWI_write_buf( uint8_t SLA, uint8_t adr, [b]uint8_t[/b] len, uint8_t *buf )
                  //  na
                 //   void TWI_write_buf( uint8_t SLA, uint8_t adr, [b]uint16_t[/b] len, uint8_t *buf )

                TWI_write_buf( SSD1306_I2C_ADDRESS, 0x40, 1024, ssd1306_buf );

        }
    //***********************************************************************************

    /**
      * FUNKCJA ZAPISUJĄCA STAN PIXALA W BUFORZE
      */
    //*************************************************************
    void SSD1306_setPixel(int x, int y, uint8_t bw )
     {
             if( (x < 0) || (x >= SSD1306_WIDTH) || (y < 0) || (y >= SSD1306_HEIGHT) )
                     return;

             if(bw )     ssd1306_buf[x + (y/8) * SSD1306_WIDTH] |=   (1<<(y%8));
             else            ssd1306_buf[x + (y/8) * SSD1306_WIDTH] &=  ~(1<<(y%8));

     }
    //*****************************************************************

    // inicjacja  wyswietlacza
    void SSD1306_init(uint8_t vcc, uint8_t refresh){

    i2cSetBitrate(400);
    I2C_DIR |= (1<<SCL)|(1<<SDA);
    _delay_ms(50);


    SSD1306_cmd(SSD1306_DISPLAYOFF);
    SSD1306_cmd(SSD1306_SETDISPLAYCLOCKDIV);
    SSD1306_cmd(refresh);

    SSD1306_cmd(SSD1306_SETDISPLAYOFFSET);
    SSD1306_cmd(0x00);

    SSD1306_cmd(SSD1306_SETSTARTLINE|0x0);
    SSD1306_cmd(SSD1306_CHARGEPUMP);

    if (vcc == SSD1306_EXTERNALVCC) SSD1306_cmd(0x10);
    else SSD1306_cmd(0x14);


    SSD1306_cmd(SSD1306_MEMORYMODE);
    SSD1306_cmd(0x00);
    SSD1306_cmd(SSD1306_SEGREMAP|0x1);
    SSD1306_cmd(SSD1306_COMSCANDEC);

    SSD1306_cmd(SSD1306_SETCONTRAST);

    if (vcc== SSD1306_EXTERNALVCC)SSD1306_cmd(0x9F);
    else SSD1306_cmd(0xCF);

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

    SSD1306_cmd(SSD1306_DISPLAYALLON_RESUME);
    SSD1306_cmd(SSD1306_NORMALDISPLAY);
    SSD1306_cmd(SSD1306_DISPLAYON);

    }

    //**********************************************************************************************
