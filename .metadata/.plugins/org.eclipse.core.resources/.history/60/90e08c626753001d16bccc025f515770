#include "tft_helpers.h"
#include "ascii.h"
#include <stdio.h>
#include <stdarg.h>
uint16_t textColor = BLUE;
uint16_t highlightColor = WHITE;

void tft_set_textColor(uint16_t color) { textColor = color; }
void tft_set_highlightColor(uint16_t color) { highlightColor = color; }

void tft_printc(uint16_t x, uint16_t y, const char cChar) {
	uint8_t ucTemp, ucRelativePositon, ucPage, ucColumn;
	ucRelativePositon = cChar - ' ';
	LCD_OpenWindow(x * CHAR_WIDTH, y * CHAR_HEIGHT, WIDTH_EN_CHAR, HEIGHT_EN_CHAR);
	LCD_Write_Cmd(CMD_SetPixel);
	for (ucPage = 0; ucPage < HEIGHT_EN_CHAR; ucPage++) {
		ucTemp = ucAscii_1608[ucRelativePositon][ucPage];
		for (ucColumn = 0; ucColumn < WIDTH_EN_CHAR; ucColumn++) {
			if (ucTemp & 0x01)
				LCD_Write_Data(textColor);
			else
				LCD_Write_Data(highlightColor);
			ucTemp >>= 1;

		}
	}
}

void tft_prints(uint16_t x, uint16_t y, const char* fmt, ...) {
	char buffer[4096];
	char *pt = buffer;
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	for (int x_ = x; *pt != 0 && x_ < MAX_X; x_++, pt++) {
		tft_printc(x_, y, *pt);
	}
}
