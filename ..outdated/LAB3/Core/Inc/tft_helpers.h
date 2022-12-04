#ifndef _TFT_HELPERS_H
#define _TFT_HELPERS_H

#include "lcd.h"

#define LCD_HEIGHT 320
#define LCD_WIDTH 240
#define CHAR_HEIGHT 16
#define CHAR_WIDTH 8
#define MAX_X 30
#define MAX_Y 20

extern uint16_t textColor;
extern uint16_t highlightColor;
void tft_set_textColor(uint16_t color);
void tft_set_highlightColor(uint16_t color);
void tft_printc(uint16_t x, uint16_t y, const char c);
void tft_prints(uint16_t x, uint16_t y, const char *fmt, ...);

#endif // _TFT_HELPERS_H
