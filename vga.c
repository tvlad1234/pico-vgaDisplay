/**
 * Hunter Adams (vha3@cornell.edu)
 *
 * VGA driver using PIO assembler
 *
 * HARDWARE CONNECTIONS
 *  - GPIO 17 ---> VGA Hsync
 *  - GPIO 16 ---> VGA Vsync
 *  - GPIO 18 ---> 330 ohm resistor ---> VGA Red
 *  - GPIO 19 ---> 330 ohm resistor ---> VGA Green
 *  - GPIO 20 ---> 330 ohm resistor ---> VGA Blue
 *  - RP2040 GND ---> VGA GND
 *
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels 0 and 1
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 *
 * Modified by Vlad Tomoiaga (tvlad1234) to be used as a library
 *
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "vga.h"

#include "hsync.pio.h"
#include "vsync.pio.h"
#include "rgb.pio.h"

#define H_ACTIVE 655   // (active + frontporch - 1) - one cycle delay for mov
#define V_ACTIVE 479   // (active - 1)
#define RGB_ACTIVE 319 // (horizontal active)/2 - 1
// #define RGB_ACTIVE 639 // change to this if 1 pixel/byte

uint16_t _width = 640;
uint16_t _height = 480;

// Pixel color array that is DMA's to the PIO machines and
// a pointer to the ADDRESS of this color array.
// Note that this array is automatically initialized to all 0's (black)
unsigned char vga_data_array[TXCOUNT];
char *address_pointer = &vga_data_array[0];

// DMA channel for dma_memcpy and dma_memset
int memcpy_dma_chan;

void VGA_initDisplay()
{
    // Choose which PIO instance to use (there are two instances, each with 4 state machines)
    PIO pio = pio1;

    // Our assembled program needs to be loaded into this PIO's instruction
    // memory. This SDK function will find a location (offset) in the
    // instruction memory where there is enough space for our program. We need
    // to remember these locations!
    //
    // We only have 32 instructions to spend! If the PIO programs contain more than
    // 32 instructions, then an error message will get thrown at these lines of code.
    //
    // The program name comes from the .program part of the pio file
    // and is of the form <program name_program>
    uint hsync_offset = pio_add_program(pio, &hsync_program);
    uint vsync_offset = pio_add_program(pio, &vsync_program);
    uint rgb_offset = pio_add_program(pio, &rgb_program);

    // Manually select a few state machines from pio instance pio0.
    uint hsync_sm = 0;
    uint vsync_sm = 1;
    uint rgb_sm = 2;

    // Call the initialization functions that are defined within each PIO file.
    // Why not create these programs here? By putting the initialization function in
    // the pio file, then all information about how to use/setup that state machine
    // is consolidated in one place. Here in the C, we then just import and use it.
    hsync_program_init(pio, hsync_sm, hsync_offset, HSYNC);
    vsync_program_init(pio, vsync_sm, vsync_offset, VSYNC);
    rgb_program_init(pio, rgb_sm, rgb_offset, RED_PIN);

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // ===========================-== DMA Data Channels =================================================
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    // DMA channels - 0 sends color data, 1 reconfigures and restarts 0
    int rgb_chan_0 = dma_claim_unused_channel(true);
    int rgb_chan_1 = dma_claim_unused_channel(true);

    // DMA channel for dma_memcpy and dma_memset
    memcpy_dma_chan = dma_claim_unused_channel(true);

    // Channel Zero (sends color data to PIO VGA machine)
    dma_channel_config c0 = dma_channel_get_default_config(rgb_chan_0); // default configs
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);             // 8-bit txfers
    channel_config_set_read_increment(&c0, true);                       // yes read incrementing
    channel_config_set_write_increment(&c0, false);                     // no write incrementing
    if (pio == pio0)
        channel_config_set_dreq(&c0, DREQ_PIO0_TX2); // DREQ_PIO0_TX2 pacing (FIFO)
    else
        channel_config_set_dreq(&c0, DREQ_PIO1_TX2); // DREQ_PIO1_TX2 pacing (FIFO)
    channel_config_set_chain_to(&c0, rgb_chan_1);    // chain to other channel

    dma_channel_configure(
        rgb_chan_0,        // Channel to be configured
        &c0,               // The configuration we just created
        &pio->txf[rgb_sm], // write address (RGB PIO TX FIFO)
        &vga_data_array,   // The initial read address (pixel color array)
        TXCOUNT,           // Number of transfers; in this case each is 1 byte.
        false              // Don't start immediately.
    );

    // Channel One (reconfigures the first channel)
    dma_channel_config c1 = dma_channel_get_default_config(rgb_chan_1); // default configs
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);            // 32-bit txfers
    channel_config_set_read_increment(&c1, false);                      // no read incrementing
    channel_config_set_write_increment(&c1, false);                     // no write incrementing
    channel_config_set_chain_to(&c1, rgb_chan_0);                       // chain to other channel

    dma_channel_configure(
        rgb_chan_1,                        // Channel to be configured
        &c1,                               // The configuration we just created
        &dma_hw->ch[rgb_chan_0].read_addr, // Write address (channel 0 read address)
        &address_pointer,                  // Read address (POINTER TO AN ADDRESS)
        1,                                 // Number of transfers, in this case each is 4 byte
        false                              // Don't start immediately.
    );

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    // Initialize PIO state machine counters. This passes the information to the state machines
    // that they retrieve in the first 'pull' instructions, before the .wrap_target directive
    // in the assembly. Each uses these values to initialize some counting registers.
    pio_sm_put_blocking(pio, hsync_sm, H_ACTIVE);
    pio_sm_put_blocking(pio, vsync_sm, V_ACTIVE);
    pio_sm_put_blocking(pio, rgb_sm, RGB_ACTIVE);

    // Start the two pio machine IN SYNC
    // Note that the RGB state machine is running at full speed,
    // so synchronization doesn't matter for that one. But, we'll
    // start them all simultaneously anyway.
    pio_enable_sm_mask_in_sync(pio, ((1u << hsync_sm) | (1u << vsync_sm) | (1u << rgb_sm)));

    // Start DMA channel 0. Once started, the contents of the pixel color array
    // will be continously DMA's to the PIO machines that are driving the screen.
    // To change the contents of the screen, we need only change the contents
    // of that array.
    dma_start_channel_mask((1u << rgb_chan_0));
}

// A function for drawing a pixel with a specified color.
// Note that because information is passed to the PIO state machines through
// a DMA channel, we only need to modify the contents of the array and the
// pixels will be automatically updated on the screen.
void VGA_writePixel(int x, int y, char color)
{
    // Range checks
    if (x > 639)
        x = 639;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (y > 479)
        y = 479;

    // Which pixel is it?
    int pixel = ((640 * y) + x);

    // Is this pixel stored in the first 3 bits
    // of the vga data array index, or the second
    // 3 bits? Check, then mask.
    if (pixel & 1)
    {
        vga_data_array[pixel >> 1] &= 0b00000111;
        vga_data_array[pixel >> 1] |= (color << 3);
    }
    else
    {
        vga_data_array[pixel >> 1] &= 0b0111000;
        vga_data_array[pixel >> 1] |= (color);
    }
}

void dma_memset(void *dest, uint8_t val, size_t num)
{
    dma_channel_config c = dma_channel_get_default_config(memcpy_dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(
        memcpy_dma_chan, // Channel to be configured
        &c,              // The configuration we just created
        dest,            // The initial write address
        &val,            // The initial read address
        num,             // Number of transfers; in this case each is 1 byte.
        true             // Start immediately.
    );

    // We could choose to go and do something else whilst the DMA is doing its
    // thing. In this case the processor has nothing else to do, so we just
    // wait for the DMA to finish.
    dma_channel_wait_for_finish_blocking(memcpy_dma_chan);
}

void dma_memcpy(void *dest, void *src, size_t num)
{
    dma_channel_config c = dma_channel_get_default_config(memcpy_dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(
        memcpy_dma_chan, // Channel to be configured
        &c,              // The configuration we just created
        dest,            // The initial write address
        src,             // The initial read address
        num,             // Number of transfers; in this case each is 1 byte.
        true             // Start immediately.
    );

    // We could choose to go and do something else whilst the DMA is doing its
    // thing. In this case the processor has nothing else to do, so we just
    // wait for the DMA to finish.
    dma_channel_wait_for_finish_blocking(memcpy_dma_chan);
}

void VGA_fillScreen(uint16_t color)
{
    dma_memset(vga_data_array, (color) | (color << 3), TXCOUNT);
}
