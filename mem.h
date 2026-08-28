#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

void mem_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
size_t mem_get_free(void);

#endif
