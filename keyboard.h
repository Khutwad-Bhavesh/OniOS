#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

char keyboard_getchar(void);
char keyboard_getchar_nowait(void);
void keyboard_readline(char* buffer, int max_len);
int keyboard_get_event(int* pressed, char* c);

#endif
