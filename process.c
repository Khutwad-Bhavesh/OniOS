#include "process.h"
#include "mem.h"
#include "vga.h"

static struct process_control_block pcb_table[MAX_PROCESSES];
static int current_pid = -1;
static uint32_t next_pid_counter = 1;

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        pcb_table[i].state = PROC_STATE_DEAD;
        pcb_table[i].pid = 0;
    }
    /* Set up main kernel process (PID 0) */
    pcb_table[0].pid = 0;
    pcb_table[0].state = PROC_STATE_RUNNING;
    
    char* name = "kernel_main";
    for(int i=0; i<31 && name[i]; i++) { pcb_table[0].name[i] = name[i]; }
    pcb_table[0].name[31] = '\0';
    
    current_pid = 0;
}

int process_create(void (*entry_point)(void), const char* name) {
    int free_slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (pcb_table[i].state == PROC_STATE_DEAD) {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1) return -1; /* No free slots */

    uint32_t stack_base = (uint32_t)kmalloc(STACK_SIZE);
    if (stack_base == 0) return -1; /* Out of memory */

    pcb_table[free_slot].pid = next_pid_counter++;
    pcb_table[free_slot].state = PROC_STATE_WAITING;
    pcb_table[free_slot].stack_base = stack_base;

    /* Setup initial stack frame for the new process */
    uint32_t* stack = (uint32_t*)(stack_base + STACK_SIZE);
    
    *(--stack) = (uint32_t)process_exit; /* Return address (if task returns) */
    *(--stack) = (uint32_t)entry_point;  /* EIP */
    *(--stack) = 0; /* EBP */
    *(--stack) = 0; /* EDI */
    *(--stack) = 0; /* ESI */
    *(--stack) = 0; /* EBX */

    pcb_table[free_slot].esp = (uint32_t)stack;

    for (int i = 0; i < 31 && name[i]; i++) {
        pcb_table[free_slot].name[i] = name[i];
    }
    pcb_table[free_slot].name[31] = '\0';

    return pcb_table[free_slot].pid;
}

/* Perform context switch using inline assembly */
__attribute__((noinline)) void process_switch(uint32_t* old_esp, uint32_t new_esp) {
    __asm__ __volatile__(
        "pushl %%ebx\n\t"
        "pushl %%esi\n\t"
        "pushl %%edi\n\t"
        "pushl %%ebp\n\t"
        "movl %%esp, (%0)\n\t" /* Save old ESP */
        "movl %1, %%esp\n\t"   /* Load new ESP */
        "popl %%ebp\n\t"
        "popl %%edi\n\t"
        "popl %%esi\n\t"
        "popl %%ebx\n\t"
        : 
        : "r"(old_esp), "r"(new_esp)
        : "memory"
    );
}

void process_yield(void) {
    if (current_pid == -1) return;

    int next_pid = -1;
    for (int i = 1; i <= MAX_PROCESSES; i++) {
        int idx = (current_pid + i) % MAX_PROCESSES;
        if (pcb_table[idx].state == PROC_STATE_WAITING || pcb_table[idx].state == PROC_STATE_RUNNING) {
            next_pid = idx;
            break;
        }
    }

    if (next_pid == -1 || next_pid == current_pid) return; /* Nothing else to run */

    int old_pid = current_pid;
    if (pcb_table[old_pid].state == PROC_STATE_RUNNING) {
        pcb_table[old_pid].state = PROC_STATE_WAITING;
    }

    current_pid = next_pid;
    pcb_table[current_pid].state = PROC_STATE_RUNNING;

    process_switch(&pcb_table[old_pid].esp, pcb_table[current_pid].esp);
}

void process_exit(void) {
    if (current_pid == 0) return; /* Kernel main cannot exit */
    
    pcb_table[current_pid].state = PROC_STATE_DEAD;
    if (pcb_table[current_pid].stack_base) {
        kfree((void*)pcb_table[current_pid].stack_base);
    }
    process_yield();
    while (1) { /* Should never reach here */ }
}

struct process_control_block* process_get_table(void) {
    return pcb_table;
}

uint32_t process_get_current_pid(void) {
    return current_pid == -1 ? 0 : pcb_table[current_pid].pid;
}
