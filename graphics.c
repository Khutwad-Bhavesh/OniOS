#include "graphics.h"
#include "font8x8.h"

uint32_t graphics_width = 0;
uint32_t graphics_height = 0;
uint32_t graphics_pitch = 0;
uint8_t graphics_bpp = 0;
static uint8_t* fb = NULL;

void graphics_init(uint64_t fb_addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp) {
    fb = (uint8_t*)(uint32_t)fb_addr; /* We are 32-bit, so cast to 32-bit pointer */
    graphics_width = width;
    graphics_height = height;
    graphics_pitch = pitch;
    graphics_bpp = bpp;
}

void graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= graphics_width || y >= graphics_height) return;
    
    uint32_t pixel_offset = y * graphics_pitch + (x * (graphics_bpp / 8));
    
    if (graphics_bpp == 32) {
        fb[pixel_offset] = color & 255;           /* B */
        fb[pixel_offset + 1] = (color >> 8) & 255;  /* G */
        fb[pixel_offset + 2] = (color >> 16) & 255; /* R */
        /* Alpha is ignored */
    } else if (graphics_bpp == 24) {
        fb[pixel_offset] = color & 255;
        fb[pixel_offset + 1] = (color >> 8) & 255;
        fb[pixel_offset + 2] = (color >> 16) & 255;
    }
    /* We don't support 16-bit or 8-bit yet for simplicity */
}

void graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            graphics_put_pixel(x + j, y + i, color);
        }
    }
}

void graphics_clear(uint32_t color) {
    graphics_draw_rect(0, 0, graphics_width, graphics_height, color);
}

void graphics_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg_color, uint32_t bg_color) {
    if (c < 0 || c >= 128) return; /* Only basic ASCII supported */
    
    char* glyph = font8x8_basic[(int)c];
    
    for (int cy = 0; cy < 8; cy++) {
        for (int cx = 0; cx < 8; cx++) {
            int set = glyph[cy] & (1 << cx);
            if (set) {
                graphics_put_pixel(x + cx, y + cy, fg_color);
            } else {
                if (bg_color != 0xFFFFFFFF) { /* Use 0xFFFFFFFF for transparent bg */
                    graphics_put_pixel(x + cx, y + cy, bg_color);
                }
            }
        }
    }
}
