add_library(vga
	vga.c
	gfx/gfx.c
)

target_include_directories(vga PUBLIC
	.
	pio
	gfx
)

target_link_libraries(vga pico_stdlib hardware_pio hardware_dma)