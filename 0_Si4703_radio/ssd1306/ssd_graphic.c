
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "ssd1306.h"

int cursor_x, cursor_y;

void SSD1306_drawLine(int x0, int y0, int x1, int y1, uint8_t color);
void SSD1306_drawFastVLine(int x, int y, int h, uint8_t color);
void SSD1306_fillRect(int x, int y, int w, int h, uint8_t color);



void SSD1306_drawRoundRect(int x, int y, int w,
        int h, int r, uint8_t color) {
    // smarter version
    SSD1306_drawFastHLine(x+r  , y    , w-2*r, color); // Top
    SSD1306_drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
    SSD1306_drawFastVLine(x    , y+r  , h-2*r, color); // Left
    SSD1306_drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    SSD1306_drawCircleHelper(x+r    , y+r    , r, 1, color);
    SSD1306_drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    SSD1306_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    SSD1306_drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

void SSD1306_drawTriangle(int x0, int y0,
        int x1, int y1, int x2, int y2, uint8_t color) {
	SSD1306_drawLine(x0, y0, x1, y1, color);
	SSD1306_drawLine(x1, y1, x2, y2, color);
	SSD1306_drawLine(x2, y2, x0, y0, color);
}



// Fill a rounded rectangle
void SSD1306_fillRoundRect(int x, int y, int w,
        int h, int r, uint8_t color) {
    // smarter version
    SSD1306_fillRect(x+r, y, w-2*r, h, color);

    // draw four corners
    SSD1306_fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    SSD1306_fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}


void SSD1306_drawCircle(int x0, int y0, int r, uint8_t color) {
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    SSD1306_setPixel(x0  , y0+r, color);
    SSD1306_setPixel(x0  , y0-r, color);
    SSD1306_setPixel(x0+r, y0  , color);
    SSD1306_setPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_setPixel(x0 + x, y0 + y, color);
        SSD1306_setPixel(x0 - x, y0 + y, color);
        SSD1306_setPixel(x0 + x, y0 - y, color);
        SSD1306_setPixel(x0 - x, y0 - y, color);
        SSD1306_setPixel(x0 + y, y0 + x, color);
        SSD1306_setPixel(x0 - y, y0 + x, color);
        SSD1306_setPixel(x0 + y, y0 - x, color);
        SSD1306_setPixel(x0 - y, y0 - x, color);
    }
}


void SSD1306_drawCircleHelper( int x0, int y0,
        int r, uint8_t cornername, uint8_t color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
        	SSD1306_setPixel(x0 + x, y0 + y, color);
        	SSD1306_setPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
        	SSD1306_setPixel(x0 + x, y0 - y, color);
        	SSD1306_setPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
        	SSD1306_setPixel(x0 - y, y0 + x, color);
        	SSD1306_setPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
        	SSD1306_setPixel(x0 - y, y0 - x, color);
        	SSD1306_setPixel(x0 - x, y0 - y, color);
        }
    }
}

void SSD1306_fillCircle(int x0, int y0, int r,
        uint8_t color) {

    SSD1306_drawFastVLine(x0, y0-r, 2*r+1, color);
    SSD1306_fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void SSD1306_fillCircleHelper(int x0, int y0, int r,
        uint8_t cornername, int delta, uint8_t color) {

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
        	SSD1306_drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
        	SSD1306_drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2) {
        	SSD1306_drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
        	SSD1306_drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}



void SSD1306_drawBitmap_P(int x, int y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color) { //może być ikonka lub na cały ekran
	int i, j, byteWidth = (w + 7) / 8;							//dopełnienie do pełnych bajtów (8) dla różnych szerokości

	for (j = 0; j < h; j++) {									// od 0 do wysokości obrazka 	/ wiersze 0-h
		for (i = 0; i < w; i++) {								// od 0 do szerokości			/ kolumny 0-w
			if (pgm_read_byte(bitmap + j * byteWidth + i / 8)	// czytaj pełen bajt z linii + bity nowej linii / ogranicz do pełnych bajtów
																// ostatni, pełny bajt w linii zapewnia byteWidth niezależnie od 'i'
					& (128 >> (i & 7))) {						// pixel i maskowany na 0-7; testowanie bajtu kolejno na pozycjach 7-0
				SSD1306_setPixel(x + i, y + j, color);			// jeśli prawda zapal piksel i w bajcie
			}
		}
	}
}


// Konkretnie dla fonyu 6x8 !!
void SSD1306_drawChar(int x, int y, char c, uint8_t color, uint8_t bg, 	uint8_t size) {

	if ((x >= SSD1306_WIDTH) || (y >= SSD1306_HEIGHT) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
		return;

	uint8_t line;
	for (int8_t i = 0; i < 6; i++) {							// Po każdej kolumnie fonta
		if (i == 5)												// Ostatnia kolumna pusta
			line = 0x0;
		else
			line = pgm_read_byte(font + (c * 5) + i);

		for (int8_t j = 0; j < 8; j++) {
			if (line & 0x1) {
				if (size == 1)
					SSD1306_setPixel(x + i, y + j, color);
				else {
					SSD1306_fillRect(x + (i * size), y + (j * size), size, size,
							color);
				}

			} else if (bg != color) {
				if (size == 1)
					SSD1306_setPixel(x + i, y + j, bg);
				else {
					SSD1306_fillRect(x + i * size, y + j * size, size, size,
							bg);
				}

			}
			line >>= 1;
		}
	}

}


void SSD1306_puts(int x, int y, char *str, uint8_t txt_size, uint8_t color, uint8_t bg) {
	cursor_x = x;
	cursor_y = y;
	while(*str) {
		SSD1306_drawChar(cursor_x, cursor_y, *str++, color, bg, txt_size);
		cursor_x += txt_size * 6;														// szerokość znaku
	}
}

void SSD1306_puts_P(int x, int y, const char *str, uint8_t txt_size, uint8_t color, uint8_t bg) {
	cursor_x = x;
	cursor_y = y;
	register char c;
	while(*str) {
		c = pgm_read_byte(str++);
		SSD1306_drawChar(cursor_x, cursor_y, c, color, bg, txt_size);
		cursor_x += txt_size * 6;														// szerokość znaku
	}
}


void SSD1306_put_int(int x, int y, int data, uint8_t txt_size, uint8_t color, uint8_t bg) {
	char buf[16];
	SSD1306_puts(x, y, itoa(data, buf, 10), txt_size, color, bg);																// szerokość znaku
}


void SSD1306_fillRect(int x, int y, int w, int h, uint8_t color) {
	for (int16_t i = x; i < x + w; i++) {
		SSD1306_drawFastVLine(i, y, h, color);
	}
}

void SSD1306_drawRect(int x, int y, int w, int h,
        uint8_t color) {
    SSD1306_drawFastHLine(x, y, w, color);
    SSD1306_drawFastHLine(x, y+h-1, w, color);
    SSD1306_drawFastVLine(x, y, h, color);
    SSD1306_drawFastVLine(x+w-1, y, h, color);
}


void SSD1306_fillScreen(uint8_t color) {
    // Update in subclasses if desired!
    SSD1306_fillRect(0, 0, SSD1306_WIDTH, SSD1306_HEIGHT, color);
}


// (x,y) is leftmost point; if unsure, calling function
// should sort endpoints or call drawLine() instead
void SSD1306_drawFastHLine(int x, int y, uint8_t w, uint8_t color) {
    // Update in subclasses if desired!
	SSD1306_drawLine(x, y, x+w-1, y, color);
}

void SSD1306_drawFastVLine(int x, int y, int h, uint8_t color) {

	SSD1306_drawLine(x, y, x, y + h - 1, color);
}

void SSD1306_drawLine(int x0, int y0, int x1, int y1, uint8_t color) {
	int steep = abs(y1-y0) > abs(x1 - x0);

	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	};

	int dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int err = dx / 2;
	int ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			SSD1306_setPixel(y0, x0, color);
		} else {
			SSD1306_setPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}

}
