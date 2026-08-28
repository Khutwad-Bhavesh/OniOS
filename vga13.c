#include "vga13.h"
#include "io.h"

static void outw_reg(uint16_t port, uint16_t val) {
    outb(port, (uint8_t)(val & 0xFF));
    outb(port + 1, (uint8_t)((val >> 8) & 0xFF));
}

void vga13_init(void) {
    /* Switch VGA hardware registers to Mode 13h (320x200 256-color graphics) */
    outb(0x3C2, 0x63);
    
    outw_reg(0x3C4, 0x0100);
    outw_reg(0x3C4, 0x0101);
    outw_reg(0x3C4, 0x0302);
    outw_reg(0x3C4, 0x0003);
    outw_reg(0x3C4, 0x0204);
    
    outw_reg(0x3D4, 0x5F00);
    outw_reg(0x3D4, 0x4F01);
    outw_reg(0x3D4, 0x5002);
    outw_reg(0x3D4, 0x8203);
    outw_reg(0x3D4, 0x5404);
    outw_reg(0x3D4, 0x8005);
    outw_reg(0x3D4, 0xBF06);
    outw_reg(0x3D4, 0x1F07);
    outw_reg(0x3D4, 0x0008);
    outw_reg(0x3D4, 0x4109);
    outw_reg(0x3D4, 0x000A);
    outw_reg(0x3D4, 0x000B);
    outw_reg(0x3D4, 0x000C);
    outw_reg(0x3D4, 0x000D);
    outw_reg(0x3D4, 0x000E);
    outw_reg(0x3D4, 0x000F);
    outw_reg(0x3D4, 0x9C10);
    outw_reg(0x3D4, 0x8E11);
    outw_reg(0x3D4, 0x8F12);
    outw_reg(0x3D4, 0x2813);
    outw_reg(0x3D4, 0x1F14);
    outw_reg(0x3D4, 0x9615);
    outw_reg(0x3D4, 0xB916);
    outw_reg(0x3D4, 0xE317);
    
    outw_reg(0x3CE, 0x0000);
    outw_reg(0x3CE, 0x0001);
    outw_reg(0x3CE, 0x0002);
    outw_reg(0x3CE, 0x0003);
    outw_reg(0x3CE, 0x0004);
    outw_reg(0x3CE, 0x4005);
    outw_reg(0x3CE, 0x0506);
    outw_reg(0x3CE, 0x0F07);
    outw_reg(0x3CE, 0xFF08);
}

void vga13_clear(uint8_t color) {
    uint8_t* const vmem = VGA13_MEMORY;
    for (int i = 0; i < VGA13_WIDTH * VGA13_HEIGHT; i++) {
        vmem[i] = color;
    }
}

void vga13_putpixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < VGA13_WIDTH && y >= 0 && y < VGA13_HEIGHT) {
        VGA13_MEMORY[y * VGA13_WIDTH + x] = color;
    }
}

void vga13_draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int ry = y; ry < y + h; ry++) {
        for (int rx = x; rx < x + w; rx++) {
            vga13_putpixel(rx, ry, color);
        }
    }
}
