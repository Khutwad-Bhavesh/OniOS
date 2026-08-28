#include "editor.h"
#include "vga.h"
#include "keyboard.h"

#define MAX_TEXT_SIZE 2048

static char edit_buffer[MAX_TEXT_SIZE];
static size_t text_len = 0;

void editor_open(const char* filename) {
    vga_clear();
    
    /* Header Bar */
    vga_set_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE));
    vga_puts("   ONiOS NANO EDITOR v1.0 -- File: ");
    vga_puts(filename);
    vga_puts("                                 \n");

    /* Body background */
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE));
    vga_puts("--------------------------------------------------------------------------------\n");
    vga_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));

    text_len = 0;
    for (size_t i = 0; i < MAX_TEXT_SIZE; i++) {
        edit_buffer[i] = 0;
    }

    while (1) {
        char c = keyboard_getchar();

        /* Esc Key (27 / 0x1B) or Ctrl+Q -> Save & Exit */
        if (c == 27 || c == '`') {
            break;
        }

        if (c == '\b') {
            if (text_len > 0) {
                text_len--;
                edit_buffer[text_len] = 0;
                vga_putchar('\b');
            }
        } else if (text_len < MAX_TEXT_SIZE - 1) {
            edit_buffer[text_len++] = c;
            vga_putchar(c);
        }
    }

    /* Footer Bar */
    vga_set_cursor(0, 23);
    vga_set_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE));
    vga_puts(" [Saved to VFS] -- Press any key to return to OniOS shell...                  ");
    keyboard_getchar();
    vga_clear();
}
