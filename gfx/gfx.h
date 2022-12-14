#ifndef _GFX_H
#define _GFX_H

#include "pico/stdlib.h"
#include "gfxfont.h"

void GFX_drawPixel(int16_t x, int16_t y, uint16_t color);

void GFX_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
void GFX_write(uint8_t c);
void GFX_setCursor(int16_t x, int16_t y);
void GFX_setTextColor(uint16_t color);
void GFX_setTextBack(uint16_t color);
void GFX_setFont(const GFXfont *f);

void GFX_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void GFX_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void GFX_drawFastHLine(int16_t x, int16_t y, int16_t l, uint16_t color);

void GFX_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GFX_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void GFX_fillScreen(uint16_t color);
void GFX_setClearColor(uint16_t color);
void GFX_clearScreen();

void GFX_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void GFX_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

void GFX_printf(const char *format, ...);
void GFX_setTextSize(uint s);

#endif
