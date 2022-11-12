#ifndef _VGA_H
#define _VGA_H

#define VGA_BGR 1

// Give the I/O pins that we're using some names that make sense
#define HSYNC 17
#define VSYNC 16
#define RED_PIN 18

// Length of the pixel array, and number of DMA transfers
#define TXCOUNT 153600 // Total pixels/2 (since we have 2 pixels per byte)

#if VGA_BGR
#define BLACK 0b0
#define RED 0b100
#define GREEN 0b010
#define YELLOW 0b110
#define BLUE 0b001
#define MAGENTA 0b101
#define CYAN 0b011
#define WHITE 0b111
#else
#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
#endif
void VGA_writePixel(int x, int y, char color);
void VGA_initDisplay();

void VGA_fillScreen(uint16_t color);

void dma_memset(void *dest, uint8_t val, size_t num);
void dma_memcpy(void *dest, void *src, size_t num);
#endif