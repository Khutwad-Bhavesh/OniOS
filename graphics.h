#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stddef.h>

void graphics_init(uint64_t fb_addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);
void graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void graphics_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color);
void graphics_draw_char_scaled(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color, int scale);
void graphics_clear(uint32_t color);

extern uint32_t graphics_width;
extern uint32_t graphics_height;
extern uint8_t graphics_bpp;

#endif
