#include "vga.h"
#include "io.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;

static void update_hardware_cursor(void) {
    uint16_t pos = vga_row * VGA_WIDTH + vga_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void vga_init(uint8_t color) {
    vga_row = 0;
    vga_column = 0;
    vga_color = color;
    vga_clear();
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
    update_hardware_cursor();
}

void vga_set_color(uint8_t color) {
    vga_color = color;
}

static void scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = VGA_MEMORY[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
    vga_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            scroll();
        }
        update_hardware_cursor();
        return;
    }

    if (c == '\b') {
        if (vga_column > 0) {
            vga_column--;
            const size_t index = vga_row * VGA_WIDTH + vga_column;
            VGA_MEMORY[index] = vga_entry(' ', vga_color);
        }
        update_hardware_cursor();
        return;
    }

    const size_t index = vga_row * VGA_WIDTH + vga_column;
    VGA_MEMORY[index] = vga_entry(c, vga_color);
    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            scroll();
        }
    }
    update_hardware_cursor();
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
