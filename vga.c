#include "vga.h"
#include "io.h"
#include "graphics.h"

#define VGA_WIDTH 50  /* 800 pixels / 16px font = 50 cols */
#define VGA_HEIGHT 37  /* 600 pixels / 16px font = 37.5 rows */

static char text_buffer[VGA_HEIGHT][VGA_WIDTH];
static uint8_t color_buffer[VGA_HEIGHT][VGA_WIDTH];

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;

static const uint32_t vga_palette[16] = {
    0x000000, /* 0: Black */
    0x0000AA, /* 1: Blue */
    0x00AA00, /* 2: Green */
    0x00AAAA, /* 3: Cyan */
    0xAA0000, /* 4: Red */
    0xAA00AA, /* 5: Magenta */
    0xAA5500, /* 6: Brown */
    0xAAAAAA, /* 7: Light Grey */
    0x555555, /* 8: Dark Grey */
    0x5555FF, /* 9: Light Blue */
    0x55FF55, /* 10: Light Green */
    0x55FFFF, /* 11: Light Cyan */
    0xFF5555, /* 12: Light Red */
    0xFF55FF, /* 13: Light Magenta */
    0xFFFF55, /* 14: Yellow */
    0xFFFFFF  /* 15: White */
};

static void redraw_char(size_t x, size_t y) {
    if (!graphics_width) return; /* Graphics not initialized yet */
    
    char c = text_buffer[y][x];
    uint8_t col = color_buffer[y][x];
    uint32_t fg = vga_palette[col & 0x0F];
    uint32_t bg = vga_palette[(col >> 4) & 0x0F];
    
    /* Draw background rect (8x16) */
    graphics_draw_rect(x * 8, y * 16, 8, 16, bg);
    /* Draw character scaled by 1, transparent bg since we just drew it */
    graphics_draw_char_scaled(c, x * 8, y * 16, fg, 0xFFFFFFFF, 1);
}

void vga_init(uint8_t color) {
    vga_row = 0;
    vga_column = 0;
    vga_color = color;
    vga_clear();
}

void vga_clear(void) {
    if (graphics_width) {
        graphics_clear(vga_palette[(vga_color >> 4) & 0x0F]);
    }
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            text_buffer[y][x] = ' ';
            color_buffer[y][x] = vga_color;
            /* Don't call redraw_char here, graphics_clear handles it */
        }
    }
    vga_row = 0;
    vga_column = 0;
}

void vga_set_color(uint8_t color) {
    vga_color = color;
}

static void scroll(void) {
    /* Shift buffers up */
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            text_buffer[y][x] = text_buffer[y + 1][x];
            color_buffer[y][x] = color_buffer[y + 1][x];
        }
    }
    /* Clear last line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        text_buffer[VGA_HEIGHT - 1][x] = ' ';
        color_buffer[VGA_HEIGHT - 1][x] = vga_color;
    }
    vga_row = VGA_HEIGHT - 1;
    
    /* Redraw entire screen */
    if (graphics_width) {
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                redraw_char(x, y);
            }
        }
    }
}

void serial_putchar(char c) {
    outb(0x3F8, c);
}

void vga_putchar(char c) {
    serial_putchar(c);
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            scroll();
        }
        return;
    }

    if (c == '\b') {
        if (vga_column > 0) {
            vga_column--;
            text_buffer[vga_row][vga_column] = ' ';
            color_buffer[vga_row][vga_column] = vga_color;
            redraw_char(vga_column, vga_row);
        }
        return;
    }

    text_buffer[vga_row][vga_column] = c;
    color_buffer[vga_row][vga_column] = vga_color;
    redraw_char(vga_column, vga_row);
    
    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            scroll();
        }
    }
}

void vga_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        vga_putchar(data[i]);
    }
}

void vga_puts(const char* data) {
    size_t len = 0;
    while (data[len]) len++;
    vga_write(data, len);
}

void vga_puthex(uint32_t val) {
    vga_puts("0x");
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        vga_putchar(hex_chars[(val >> i) & 0xF]);
    }
}

void vga_putdec(uint32_t val) {
    if (val == 0) {
        vga_putchar('0');
        return;
    }
    char buf[12];
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (--i >= 0) {
        vga_putchar(buf[i]);
    }
}

void vga_set_cursor(size_t x, size_t y) {
    if (x < VGA_WIDTH) vga_column = x;
    if (y < VGA_HEIGHT) vga_row = y;
}


