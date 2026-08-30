#include "idt.h"
#include "io.h"
#include "vga.h"

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

extern void idt_load(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

static void pic_remap(void) {
    /* Remap Master & Slave 8259 PIC to IRQ 32..47 */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master PIC offset = 32 (0x20)
    outb(0xA1, 0x28); // Slave PIC offset = 40 (0x28)
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFE); // Enable ONLY IRQ0 (bit 0 is 0)
    outb(0xA1, 0xFF); // Disable all Slave IRQs
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    pic_remap();

    /* Load IDT register using asm */
    __asm__ __volatile__ ("lidt %0" : : "m" (idtp));
}
