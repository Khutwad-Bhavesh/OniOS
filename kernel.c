#include "vga.h"
#include "keyboard.h"
#include "io.h"

#ifdef HAS_RUST
/* Rust Memory Safety Subsystem Externs */
extern int rust_security_check(void);
extern const char* rust_get_status(void);
#endif

static int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void print_banner(void) {
    /* Turbo C Classic Yellow on Blue Header */
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE));
    vga_puts("================================================================================\n");
    vga_puts("   OniOS v1.0 -- Bare-Metal Unix-like Kernel [GTO Edition]                       \n");
    vga_puts("   \"Lessons in Operating Systems -- Never follow boring rules!\"           \n");
    vga_puts("================================================================================\n");
    
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
#ifdef HAS_RUST
    vga_puts("  Architecture : x86 32-bit Multiboot Kernel (C + ASM + Rust)\n");
#else
    vga_puts("  Architecture : x86 32-bit Multiboot Kernel (C + ASM)\n");
#endif
    vga_puts("  Video Driver : 80x25 VGA Text Memory (0xB8000)\n");
    vga_puts("  Input Driver : PS/2 Keyboard Hardware Port (0x60)\n");
#ifdef HAS_RUST
    if (rust_security_check() == 1) {
        vga_puts((const char*)rust_get_status());
    }
#else
    vga_puts("  [C Core]     : Standard Bare-Metal C Subsystem Active\n");
#endif
    vga_puts("  Status       : Standalone Bare-Metal (No Linux / No POSIX)\n");
    vga_puts("================================================================================\n\n");
}

void shell_run(void) {
    char cmd[128];
    while (1) {
        /* Shell Prompt in Bright Green on Blue */
        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE));
        vga_puts("OniOS> ");

        /* User input in Light Grey */
        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE));
        keyboard_readline(cmd, 128);

        /* Command execution */
        if (strcmp(cmd, "help") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
            vga_puts("Available OniOS Commands:\n");
            vga_puts("  help     - Show available kernel commands\n");
            vga_puts("  gto      - Display GTO ASCII quote & lesson\n");
            vga_puts("  cresta   - Vice Principal Uchiyamada's Cresta status\n");
            vga_puts("  suplex   - Execute German Suplex on system bugs\n");
            vga_puts("  info     - Display hardware & memory info\n");
            vga_puts("  clear    - Clear screen buffer\n");
            vga_puts("  reboot   - Hardware CPU reboot via keyboard controller\n");
        } 
        else if (strcmp(cmd, "gto") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE));
            vga_puts("\n  \"I'm Eikichi Onizuka, 22 years old, single! Welcome to OniOS!\"\n");
            vga_puts("  Lesson 1: If an OS crashes, give it a German Suplex!\n\n");
        } 
        else if (strcmp(cmd, "cresta") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE));
            vga_puts("  [!] BOOM! Vice Principal Uchiyamada's Cresta just got wrecked again! 🚗💨\n");
        } 
        else if (strcmp(cmd, "suplex") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLUE));
            vga_puts("  [!] EIKICHI ONIZUKA DELIVERS A GERMAN SUPLEX TO THE KERNEL BUG! 🤼‍♂️💥\n");
        } 
        else if (strcmp(cmd, "info") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE));
            vga_puts("  OS Name    : OniOS Bare-Metal Kernel\n");
            vga_puts("  VGA Address: ");
            vga_puthex(0xB8000);
            vga_puts("\n  Keyboard IO: Port 0x60 / Status 0x64\n");
        } 
        else if (strcmp(cmd, "clear") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE));
            vga_clear();
            print_banner();
        } 
        else if (strcmp(cmd, "reboot") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE));
            vga_puts("Rebooting hardware CPU...\n");
            /* Pulse CPU reset line via PS/2 keyboard controller */
            uint8_t good = 0x02;
            while (good & 0x02) {
                good = inb(0x64);
            }
            outb(0x64, 0xFE);
        } 
        else if (cmd[0] != '\0') {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE));
            vga_puts("Unknown command: ");
            vga_puts(cmd);
            vga_puts(" (type 'help' for commands)\n");
        }
    }
}

void kernel_main(void) {
    /* Set Turbo C style: Light Grey text on Dark Blue background */
    vga_init(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE));
    print_banner();
    shell_run();
}
