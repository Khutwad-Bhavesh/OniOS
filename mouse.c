#include "mouse.h"
#include "io.h"
#include "vga.h"

/* Mouse State Variables */
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
static int32_t mouse_x = 40; /* Start in middle of 80x25 screen */
static int32_t mouse_y = 12;

/* Store character underneath cursor */
static uint16_t old_mouse_char = 0;

static uint16_t* vga_buffer = (uint16_t*) 0xB8000;

/* Wait for the mouse to be ready to read or write */
static void mouse_wait(uint8_t a_type) {
    uint32_t timeout = 100000;
    if (a_type == 0) {
        while (timeout--) {
            if ((inb(MOUSE_CMD_PORT) & 1) == 1) {
                return;
            }
        }
    } else {
        while (timeout--) {
            if ((inb(MOUSE_CMD_PORT) & 2) == 0) {
                return;
            }
        }
    }
}

static void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0xD4);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, a_write);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

void mouse_init(void) {
    uint8_t status;

    /* Enable auxiliary mouse device */
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0xA8);
    
    /* Enable interrupts (IRQ12) */
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x20);
    mouse_wait(0);
    status = (inb(MOUSE_DATA_PORT) | 2);
    mouse_wait(1);
    outb(MOUSE_CMD_PORT, 0x60);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);
    
    /* Set default settings */
    mouse_write(0xF6);
    mouse_read();
    
    /* Enable data reporting */
    mouse_write(0xF4);
    mouse_read();

    /* Save initial character */
    old_mouse_char = vga_buffer[mouse_y * 80 + mouse_x];

    /* Setup IDT Gate 44 (IRQ12) */
    extern void irq12_handler_asm(void);
    extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
    idt_set_gate(44, (uint32_t)irq12_handler_asm, 0x10, 0x8E);

    vga_puts("[Mouse] PS/2 Mouse Initialized (IRQ12 Enabled)\n");
}

/* 
 * The main interrupt handler.
 * Called from irq12.s
 */
void mouse_handler(void) {
    /* Read byte from mouse */
    uint8_t status = inb(MOUSE_CMD_PORT);
    while (status & 0x01) {
        int8_t mouse_in = inb(MOUSE_DATA_PORT);
        
        switch(mouse_cycle) {
            case 0:
                mouse_byte[0] = mouse_in;
                if (!(mouse_in & 0x08)) break; /* Error out of sync */
                mouse_cycle++;
                break;
            case 1:
                mouse_byte[1] = mouse_in;
                mouse_cycle++;
                break;
            case 2:
                mouse_byte[2] = mouse_in;
                
                /* Calculate deltas */
                int delta_x = mouse_byte[1];
                int delta_y = mouse_byte[2];
                
                /* Adjust for sign bits in byte 0 */
                int x_sign = mouse_byte[0] & 0x10;
                int y_sign = mouse_byte[0] & 0x20;
                if (x_sign) delta_x |= 0xFFFFFF00; /* Sign extend */
                if (y_sign) delta_y |= 0xFFFFFF00;

                /* Restore old character underneath */
                vga_buffer[mouse_y * 80 + mouse_x] = old_mouse_char;

                /* Update positions */
                mouse_x += delta_x;
                mouse_y -= delta_y; /* Y axis is reversed on screen */

                /* Clamp to screen (80x25) */
                if (mouse_x < 0) mouse_x = 0;
                if (mouse_x > 79) mouse_x = 79;
                if (mouse_y < 0) mouse_y = 0;
                if (mouse_y > 24) mouse_y = 24;

                /* Handle Clicks (Debug printing, not real UI interaction yet) */
                if (mouse_byte[0] & 0x01) {
                    /* Left click */
                    // Can implement custom actions here
                }
                if (mouse_byte[0] & 0x02) {
                    /* Right click */
                    // Can implement custom actions here
                }

                /* Save new character and draw block cursor */
                old_mouse_char = vga_buffer[mouse_y * 80 + mouse_x];
                /* Draw cursor (e.g. Yellow background) */
                vga_buffer[mouse_y * 80 + mouse_x] = (old_mouse_char & 0x00FF) | (0xE0 << 8);

                mouse_cycle = 0;
                break;
        }
        status = inb(MOUSE_CMD_PORT);
    }
    
    /* EOI (End of Interrupt) to Master and Slave PICs */
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
