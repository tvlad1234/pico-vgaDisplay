# pico-vgaDisplay
*VGA display driver library for RP2040 pico-sdk* \
*based on the [PIO Assembly VGA Driver for RP2040 by V. Hunter Adams](https://vanhunteradams.com/Pico/VGA/VGA.html)*
## Library usage
Add the *pico-vgaDisplay* subdirectory to the CMakeLists.txt of your project and add *vga* in *target_link_libraries*

### Driver usage:
Include _vga.h_ in your source file. The VGA display is initialized by calling `VGA_initDisplay(uint vsync_pin, uint hsync_pin, uint r_pin);`. This also sets the used pins Uncomment `#define VGA_BGR 1` if you want to use the display in BGR mode instead of RGB. 

Pixels are written to the display with the `VGA_writePixel(int x, int y, char color);` function. There are 8 available colours (red, green, blue, cyan, magenta, yellow, black and white).

The screen can be filled with a single colour using `VGA_fillScreen(uint16_t color);`

The library also includes the `dma_memset(void *dest, uint8_t val, size_t num);` and `dma_memcpy(void *dest, void *src, size_t num);` functions. They work like the *memset* and *memcpy*, except they use the DMA hardware of the RP2040 to copy data and are much faster.

### GFX Library usage:
This package provides a graphics library, based on [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library). You can use it by including _gfx.h_ in your source file.
It supports drawing basic shapes, characters and using custom fonts.

## GFX Library Reference
`GFX_drawPixel(int16_t x, int16_t y, uint16_t color);` draws a single pixel
### 
`GFX_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
                          uint16_t bg, uint8_t size_x, uint8_t size_y);` puts a single character on screen\
`GFX_write(uint8_t c);` writes a character to the screen, handling the cursor position and text wrapping automatically\
`GFX_setCursor(int16_t x, int16_t y);` places the text cursor at the specified coordinates\
`GFX_setTextColor(uint16_t color);` sets the text color\
`GFX_setTextBack(uint16_t color);` sets the text background color\
`GFX_setFont(const GFXfont *f);`  sets the used font, using the same format as [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library) \
`GFX_printf` prints formatted text
###
`GFX_fillScreen(uint16_t color);` fills the screen with a specified color\
`GFX_setClearColor(uint16_t color);` sets the color the screen should be cleared with\
`GFX_clearScreen();` clears the screen, filling it with the color specified using the function above
###
`GFX_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);` draws a line from (x0,y0) to (x1,y1)\
`GFX_drawFastHLine(int16_t x, int16_t y, int16_t l, uint16_t color);` draws a horizontal line\
`GFX_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);` draws a vertical line
###
`GFX_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);` draws a rectangle\
`GFX_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)` draws a circle\
`GFX_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);` draws a filled rectangle\
`GFX_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);` draws a filled circle
