#include "timer.h"
#include "io.h"
#include "audio.h"

static volatile uint32_t timer_ticks = 0;
static uint32_t timer_freq = 100;

void timer_handler(void) {
    timer_ticks++;
    
    /* Process audio sequencer tick */
    audio_tick();

    /* Send End of Interrupt (EOI) signal to Master PIC */
    outb(0x20, 0x20);
}

extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
extern void irq0_handler_asm(void);

void timer_init(uint32_t frequency) {
    timer_freq = frequency;
    uint32_t divisor = 1193182 / frequency;

    /* Write PIT 8254 Control Word: Channel 0, Square Wave Mode */
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
    
    idt_set_gate(32, (uint32_t)irq0_handler_asm, 0x10, 0x8E);
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

uint32_t timer_get_uptime_ms(void) {
    return (timer_ticks * 1000) / timer_freq;
}

void timer_sleep(uint32_t ms) {
    uint32_t start = timer_get_uptime_ms();
    while (timer_get_uptime_ms() - start < ms) {
        /* Busy wait hardware loop until tick arrives */
        __asm__ __volatile__ ("nop");
    }
}
