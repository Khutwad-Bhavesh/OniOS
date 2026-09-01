#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stddef.h>

/* PS/2 Mouse Ports */
#define MOUSE_DATA_PORT   0x60
#define MOUSE_CMD_PORT    0x64

/* Function Prototypes */
void mouse_init(void);
void mouse_handler(void);

#endif /* MOUSE_H */
