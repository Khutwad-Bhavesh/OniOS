#ifndef VGA13_H
#define VGA13_H

#include <stdint.h>
#include <stddef.h>

#define VGA13_WIDTH 320
#define VGA13_HEIGHT 200
#define VGA13_MEMORY ((uint8_t*)0xA0000)

void vga13_init(void);
void vga13_clear(uint8_t color);
void vga13_putpixel(int x, int y, uint8_t color);
void vga13_draw_rect(int x, int y, int w, int h, uint8_t color);

#endif
