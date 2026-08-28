#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(uint32_t frequency);
void timer_handler(void);
uint32_t timer_get_ticks(void);
uint32_t timer_get_uptime_ms(void);
void timer_sleep(uint32_t ms);

#endif
