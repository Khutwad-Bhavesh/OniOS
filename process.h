#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PROCESSES 8
#define STACK_SIZE 4096

typedef enum {
    PROC_STATE_DEAD,
    PROC_STATE_RUNNING,
    PROC_STATE_WAITING
} process_state_t;

struct process_control_block {
    uint32_t pid;
    char name[32];
    process_state_t state;
    uint32_t esp;     /* Saved stack pointer */
    uint32_t stack_base; /* Base of allocated stack */
};

/* Initialize the process scheduler */
void process_init(void);

/* Create a new lightweight process (Ring 0 thread) */
int process_create(void (*entry_point)(void), const char* name);

/* Yield CPU to the next ready process */
void process_yield(void);

/* Exit the current process */
void process_exit(void);

/* Get list of active processes for 'ps' command */
struct process_control_block* process_get_table(void);

/* Get current PID */
uint32_t process_get_current_pid(void);

#endif
