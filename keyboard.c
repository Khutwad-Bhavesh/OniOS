#include "keyboard.h"
#include "io.h"
#include "vga.h"

/* US QWERTY Scancode Map (Set 1) */
static const char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',   0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

char keyboard_getchar(void) {
    while (1) {
        /* Check if keyboard buffer has data (status register port 0x64, bit 0) */
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            /* Only process key press (ignore key release where bit 7 is set) */
            if (!(scancode & 0x80)) {
                char c = scancode_map[scancode];
                if (c != 0) {
                    return c;
                }
            }
        }
    }
}

char keyboard_getchar_nowait(void) {
    if (inb(0x64) & 1) {
        uint8_t scancode = inb(0x60);
        if (!(scancode & 0x80)) {
            char c = scancode_map[scancode];
            if (c != 0) {
                return c;
            }
        }
    }
    return 0;
}

void keyboard_readline(char* buffer, int max_len) {
    int pos = 0;
    while (1) {
        char c = keyboard_getchar();
        if (c == '\n') {
            vga_putchar('\n');
            buffer[pos] = '\0';
            return;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                vga_putchar('\b');
            }
        } else if (c >= ' ' && c <= '~') {
            if (pos < max_len - 1) {
                buffer[pos++] = c;
                vga_putchar(c);
            }
        }
    }
}

int keyboard_get_event(int* pressed, char* c) {
    if (inb(0x64) & 1) {
        uint8_t scancode = inb(0x60);
        *pressed = !(scancode & 0x80);
        char ch = scancode_map[scancode & 0x7F];
        if (ch != 0) {
            *c = ch;
            return 1;
        }
    }
    return 0;
}
