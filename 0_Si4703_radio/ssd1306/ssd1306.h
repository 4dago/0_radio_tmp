
#ifndef SSD1306_H_
#define SSD1306_H_

#define SSD1306_128_64  		//definicja wielkosci wyswietlacza gdy nie uzywany to w komentarz
//#define SSD1306_128_32        //definicja wielkosci wyswietlacza

#define USE_CS                  0               // jak używam to 1
#define USE_RST                 0

//#define USE_SPI_OR_I2C                0       // 1-SPI ,0-I2C

/**
 * KONFIGURACJA SPRZĘTOWEGO I2C

 */
//#if USE_SPI_OR_I2C==0
// NA ARDUINO WYŚWIETLACZ PRACUJE POD ADRESEM 0x3C mimo iż na obudowie jest napisane 0x78
#define SSD1306_I2C_ADDRESS 0x78
//#define SSD1306_I2C_ADDRESS 0x3D
//#define SSD1306_I2C_ADDRESS 0x7A
//#define SSD1306_I2C_ADDRESS 0x78

#define I2C_DIR             	DDRC
#define SCL             		PC5
#define SDA             		PC4
//#endif

#define SSD1306_WIDTH   128                     		// szerokosc wyswietlacza

#if defined SSD1306_128_64                     			// wysokosc jeśli wybieramy
#define SSD1306_HEIGHT  64

#endif
#if defined SSD1306_128_32
#define SSD1306_HEIGHT  32
#endif

#define BUF_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT/8)	// bufor obrazu w ram

/**
 * DEFINICJE SZYBKOŚCI ODŚWIEŻANIA
 */
#define REFRESH_MIN     		0x80
#define REFRESH_MID     		0xB0
#define REFRESH_MAX         	0xF0

typedef enum {page0, page1, page2, page3, page4, page5, page6, page7} TPAGE;


        /**
         * DEFINICJE KOMENDY STEROWNIKA
         */
        #define SSD1306_SETCONTRAST                         0x81
        #define SSD1306_DISPLAYALLON_RESUME                         0xA4
        #define SSD1306_DISPLAYALLON                            0xA5
        #define SSD1306_NORMALDISPLAY                       0xA6
        #define SSD1306_INVERTDISPLAY                       0xA7
        #define SSD1306_DISPLAYOFF                      0xAE
        #define SSD1306_DISPLAYON                       0xAF
        #define SSD1306_SETDISPLAYOFFSET                    0xD3
        #define SSD1306_SETCOMPINS                      0xDA
        #define SSD1306_SETVCOMDETECT                       0xDB
        #define SSD1306_SETDISPLAYCLOCKDIV                  0xD5
        #define SSD1306_SETPRECHARGE                                0xD9
        #define SSD1306_SETMULTIPLEX                        0xA8
        #define SSD1306_SETLOWCOLUMN                        0x00
        #define SSD1306_SETHIGHCOLUMN                       0x10
        #define SSD1306_SETSTARTLINE                        0x40
        #define SSD1306_MEMORYMODE                      0x20
        #define SSD1306_COLUMNADDR                      0x21
        #define SSD1306_PAGEADDR                        0x22
        #define SSD1306_COMSCANINC                      0xC0
        #define SSD1306_COMSCANDEC                      0xC8
        #define SSD1306_SEGREMAP                        0xA0 // uwaga, może być A1 00-lewy gorny rog / w dalej kodzie jest
        #define SSD1306_CHARGEPUMP                      0x8D
        #define SSD1306_EXTERNALVCC                         0x1
        #define SSD1306_SWITCHCAPVCC                        0x2
        #define SSD1306_ACTIVATE_SCROLL                     0x2F
        #define SSD1306_DEACTIVATE_SCROLL                   0x2E
        #define SSD1306_SET_VERTICAL_SCROLL_AREA                                0xA3
        #define SSD1306_RIGHT_HORIZONTAL_SCROLL                                 0x26
        #define SSD1306_LEFT_HORIZONTAL_SCROLL                                  0x27
        #define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL    0x29
        #define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL     0x2A


    #define swap(a,b) {int16_t t = a ; a = b ; b = t; }


    /*********************deklaracja zmiennych
 */
extern const uint8_t font[];
//extern const uint8_t font2[];		// polskie win
extern int cursor_x, cursor_y;
/**
 * DEKLARACJE FUNKCJI
 */

void SSD1306_init(uint8_t vcc, uint8_t refresh);
void SSD1306_cmd(uint8_t cmd);
void SSD1306_data(uint8_t data);

void SSD1306_display(void);                           		// wyslij na ekran z bufor
void SSD1306_refreshPages (uint8_t page_start, uint8_t page_end, uint8_t col_start, uint8_t col_end);
void SSD1306_setPixel(int x, int y, uint8_t color);     		// zapal pizel w buforze
void SSD1306_cls(void);                         				// czyszczenie zawartosci bufora

//funkcje graficzne
void SSD1306_drawRoundRect(int x, int y, int w, int h, int r, uint8_t color);
void SSD1306_fillRoundRect(int x, int y, int w, int h, int r, uint8_t color);
void SSD1306_drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color);
void SSD1306_drawCircle(int x0, int y0, int r, uint8_t color);
void SSD1306_drawCircleHelper( int x0, int y0, int r, uint8_t cornername, uint8_t color);
void SSD1306_fillCircle(int x0, int y0, int r, uint8_t color);
void SSD1306_fillCircleHelper(int x0, int y0, int r, uint8_t cornername, int delta, uint8_t color);
void SSD1306_fillScreen(uint8_t color);
void SSD1306_fillRect(int x, int y, int w, int h, uint8_t color);
void SSD1306_drawRect(int x, int y, int w, int h, uint8_t color);



void SSD1306_drawBitmap_P(int x, int y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color); // rysowanie bitmapy
void SSD1306_drawKnownBitmap_P(int x, int y, const uint8_t *bitmap, uint8_t color);
void SSD1306_drawChar(int x, int y, char c, uint8_t color, uint8_t bg, uint8_t size); // wysowanie litery y lewy gorny róg litery kod asci litery 'A', kolor =1 , tło= 0 , rozmiar standard 1
void SSD1306_fillRect(int x, int y, int w, int h, uint8_t color); // rysowanie prostokata x,y górny lewy róg prostokata, w szerokosc , h wysokosc
void SSD1306_drawFastVLine(int x, int y, int h, uint8_t color);
void SSD1306_drawFastHLine(int x, int y, uint8_t w, uint8_t color);
void SSD1306_drawLine(int x0, int y0, int x1, int y1, uint8_t color); // rysowanie linii
void SSD1306_puts(int x, int y, char *str, uint8_t txt_size, uint8_t color, uint8_t bg);
void SSD1306_puts_P(int x, int y, const char *str, uint8_t txt_size, uint8_t color, uint8_t bg);
void SSD1306_put_int(int x, int y, int data, uint8_t txt_size, uint8_t color, uint8_t bg);


#endif /* SSD1306_H_ */

