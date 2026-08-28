#include "mem.h"

#define HEAP_SIZE (1024 * 1024) /* 1MB Dynamic Memory Heap */

static uint8_t heap[HEAP_SIZE];
static size_t heap_used = 0;

void mem_init(void) {
    heap_used = 0;
    for (size_t i = 0; i < HEAP_SIZE; i++) {
        heap[i] = 0;
    }
}

void* kmalloc(size_t size) {
    /* Align allocations to 8-byte boundaries */
    size = (size + 7) & ~7;
    if (heap_used + size > HEAP_SIZE) {
        return NULL; /* Out of Memory */
    }
    void* ptr = &heap[heap_used];
    heap_used += size;
    return ptr;
}

void kfree(void* ptr) {
    (void)ptr;
    /* Basic bump allocator: memory freed on heap reset */
}

size_t mem_get_free(void) {
    return HEAP_SIZE - heap_used;
}
