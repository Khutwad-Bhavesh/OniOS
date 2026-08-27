#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

char keyboard_getchar(void);
void keyboard_readline(char* buffer, int max_len);

#endif
