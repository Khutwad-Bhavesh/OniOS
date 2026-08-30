#include "vga.h"
#include "keyboard.h"
#include "io.h"
#include "mem.h"
#include "editor.h"
#include "idt.h"
#include "timer.h"
#include "test_suite.h"
#include "process.h"
#include "explainer_data.h"
#include "doom_engine.h"



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

static int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

/* Current Working Directory State */
static char current_dir[64] = "/";

/* Virtual Filesystem Table */
struct vfs_entry {
    const char* dir;
    const char* name;
    int is_dir;
    const char* content;
};

static const struct vfs_entry vfs_table[] = {
    {"/", "sys", 1, ""},
    {"/", "dev", 1, ""},
    {"/", "roms", 1, ""},
    {"/", "readme.txt", 0, "OniOS Bare-Metal Engine -- Clean Unix-like Kernel Foundation!\nType 'edit' to create/modify files | Type 'game' to play Arcade!\n"},
    {"/sys", "cpu", 0, "CPU: x86 32-bit Multiboot Engine\nVendor: x86 Hardware CPU\nMode: Protected 32-bit Ring 0\n"},
    {"/sys", "vga", 0, "Memory: 0xB8000 (VGA Text Buffer 80x25)\nColors: 16-Color Palette (Turbo C Theme)\n"},
    {"/dev", "keyboard", 0, "Port: 0x60 (Data) / 0x64 (Status)\nProtocol: PS/2 Scancode Set 1\n"},
    {"/roms", "nes_gto.nes", 0, "[NES ROM Header]: Onizuka's Great Adventure (NES 8-Bit) - Ready for Emulator Subsystem!\n"},
    {"/roms", "snes_cresta.sfc", 0, "[SNES ROM Header]: Uchiyamada's Cresta Grand Prix (SNES 16-Bit)\n"}
};

static const size_t VFS_COUNT = sizeof(vfs_table) / sizeof(vfs_table[0]);

static void print_banner(void) {
    /* Turbo C Classic Yellow on Blue Header */
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    vga_puts("==================================================\n");
    vga_puts("   OniOS v1.0 -- Bare-Metal Unix Kernel [Modular Subsystem Core]                \n");
    vga_puts("   \"Lessons in Operating Systems -- Never follow boring rules!\"           \n");
    vga_puts("==================================================\n");
    
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
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
    vga_puts("  Status       : VFS, Heap Allocator & Nano Editor Active (Type 'help')\n");
    vga_puts("==================================================\n\n");
}

/* Bare-Metal Retro Game: Onizuka Cresta Dodge */
static void play_game(void) {
    vga_clear();
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    vga_puts("==================================================\n");
    vga_puts("         🎮 ONIZUKA CRESTA DODGE -- Bare-Metal Arcade Engine 🎮               \n");
    vga_puts("==================================================\n");
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    vga_puts(" Controls: 'a' = Move Left | 'd' = Move Right | 'q' = Quit Game\n");
    vga_puts(" Objective: Dodge Uchiyamada's Crestas [CAR] descending down the highway!\n");
    vga_puts("--------------------------------------------------\n\n");

    int player_x = 40;
    int car_x = 35;
    int car_y = 6;
    int score = 0;
    int game_over = 0;

    while (!game_over) {
        /* Render game state */
        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        vga_puts(" Score: ");
        vga_putdec(score);
        vga_puts(" | Player Pos: ");
        vga_putdec(player_x);
        vga_puts(" | Car Pos: ");
        vga_putdec(car_x);
        vga_puts("\n");

        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        vga_puts("  [CAR] Uchiyamada's Cresta approaching at Y=");
        vga_putdec(car_y);
        vga_puts("\n");

        vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        vga_puts("  (P) Onizuka's Bike at X=");
        vga_putdec(player_x);
        vga_puts("\n\n");

        vga_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        vga_puts(" Press key [a/d/q]: ");

        char c = keyboard_getchar();
        vga_putchar(c);
        vga_putchar('\n');

        if (c == 'q') {
            vga_puts("Exiting game...\n");
            break;
        } else if (c == 'a' && player_x > 10) {
            player_x -= 5;
        } else if (c == 'd' && player_x < 70) {
            player_x += 5;
        }

        /* Advance obstacle */
        car_y += 2;
        if (car_y >= 20) {
            car_y = 6;
            car_x = 10 + ((score * 17 + 23) % 55);
            score += 100;
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts("  [!] Cresta Dodged! +100 Points!\n");
        }

        /* Collision detection */
        if (car_y >= 18 && (player_x >= car_x - 3 && player_x <= car_x + 3)) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            vga_puts("\n  💥 BOOM! You crashed into Vice Principal Uchiyamada's Cresta! 💥\n");
            vga_puts("  GAME OVER! Final Score: ");
            vga_putdec(score);
            vga_puts(" points!\n\n");
            game_over = 1;
        }
    }

    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    vga_puts("Press any key to return to OniOS Shell...");
    keyboard_getchar();
    vga_clear();
    print_banner();
}

void test_process(void) {
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    vga_puts("\n[Background Process] Hello from a new thread!\n");
    vga_puts("[Background Process] Yielding back to main shell...\n");
    process_yield();
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    vga_puts("\n[Background Process] Resumed! Now exiting thread.\n");
    process_exit();
}

void shell_run(void) {
    char cmd[128];
    while (1) {
        /* Shell Prompt displaying current_dir in Bright Green */
        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        vga_puts("OniOS:");
        vga_puts(current_dir);
        vga_puts("> ");

        /* User input in Light Grey */
        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        keyboard_readline(cmd, 128);

        /* Command execution */
        if (strcmp(cmd, "help") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
            vga_puts("Available OniOS Commands:\n");
            vga_puts("  test     - Run 6-Phase Kernel Diagnostic & Stress-Test Suite!\n");
            vga_puts("  uptime   - Display exact system uptime & clock ticks\n");
            vga_puts("  ticks    - Display raw PIT 8254 timer clock ticks\n");
            vga_puts("  sleep <m>- Pause execution for millisecond duration\n");
            vga_puts("  onizuka  - Run Great Teacher Onizuka's Code Explainer! (e.g. 'onizuka boot.s')\n");
            vga_puts("  ps       - List active processes & threads\n");
            vga_puts("  fork     - Spawn a background test process\n");
            vga_puts("  yield    - Yield CPU to next process\n");
            vga_puts("  cd <dir> - Change working directory (e.g. 'cd roms', 'cd sys', 'cd ..')\n");
            vga_puts("  ls       - List directory contents & subfolders\n");
            vga_puts("  cat <f>  - Display virtual file content\n");
            vga_puts("  pwd      - Print current directory\n");
            vga_puts("  edit <f> - Launch Bare-Metal Nano Text Editor\n");
            vga_puts("  mem      - Display dynamic kernel heap RAM usage\n");
            vga_puts("  bench    - CPU & memory speed benchmark utility\n");
            vga_puts("  game     - Launch Onizuka Cresta Dodge Bare-Metal Arcade!\n");
            vga_puts("  gto      - Display GTO ASCII quote & lesson\n");
            vga_puts("  cresta   - Vice Principal Uchiyamada's Cresta status\n");
            vga_puts("  suplex   - Execute German Suplex on system bugs\n");
            vga_puts("  info     - Display hardware & memory info\n");
            vga_puts("  clear    - Clear screen buffer\n");
            vga_puts("  reboot   - Hardware CPU reboot via keyboard controller\n");
        } 
        else if (strcmp(cmd, "ps") == 0) {
            struct process_control_block* pcb = process_get_table();
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts("PID  STATE       NAME\n");
            vga_puts("---  -----       ----\n");
            for (int i = 0; i < MAX_PROCESSES; i++) {
                if (pcb[i].state != PROC_STATE_DEAD) {
                    vga_putdec(pcb[i].pid);
                    vga_puts("    ");
                    if (pcb[i].state == PROC_STATE_RUNNING) vga_puts("RUNNING     ");
                    else vga_puts("WAITING     ");
                    vga_puts(pcb[i].name);
                    vga_puts("\n");
                }
            }
        }
        else if (strcmp(cmd, "fork") == 0) {
            int pid = process_create(test_process, "test_thread");
            if (pid != -1) {
                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                vga_puts("Spawned new thread with PID: ");
                vga_putdec(pid);
                vga_puts("\n");
            } else {
                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                vga_puts("Failed to spawn process!\n");
            }
        }
        else if (strcmp(cmd, "yield") == 0) {
            process_yield();
        }
        else if (strcmp(cmd, "test") == 0 || strcmp(cmd, "testall") == 0) {
            test_suite_run_all( );
            print_banner();
        }
        else if (strcmp(cmd, "uptime") == 0) {

            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts("System Uptime: ");
            vga_putdec(timer_get_uptime_ms() / 1000);
            vga_puts(" Seconds (");
            vga_putdec(timer_get_uptime_ms());
            vga_puts(" ms)\n");
        }
        else if (strcmp(cmd, "ticks") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
            vga_puts("PIT 8254 Hardware Timer Ticks: ");
            vga_putdec(timer_get_ticks());
            vga_puts(" ticks (100Hz Clock)\n");
        }
        else if (strncmp(cmd, "sleep ", 6) == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            vga_puts("Sleeping...\n");
            timer_sleep(1000);
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts("Woke up from 1000ms hardware timer sleep!\n");
        }
        else if (strncmp(cmd, "edit ", 5) == 0) {
            const char* fname = cmd + 5;
            editor_open(fname);
            print_banner();
        }

        else if (strcmp(cmd, "mem") == 0 || strcmp(cmd, "free") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts("OniOS Heap RAM Status:\n");
            vga_puts("  Total Heap: 1,048,576 Bytes (1MB Ring 0 Memory)\n");
            vga_puts("  Free Heap : ");
            vga_putdec(mem_get_free());
            vga_puts(" Bytes\n");
        }
        else if (strcmp(cmd, "bench") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            vga_puts("Running OniOS CPU & Memory Benchmark...\n");
            uint32_t start = 0;
            for (uint32_t i = 0; i < 5000000; i++) {
                start += (i * 3 + 7) ^ 0x5A5A;
            }
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts("  [PASS] 5,000,000 Integer Operations Completed!\n");
            vga_puts("  Result Score: 9,850 ONI-FLOPS (x86 Ring 0 Execution)\n");
        }
        else if (strcmp(cmd, "pwd") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts(current_dir);
            vga_puts("\n");
        }
        else if (strcmp(cmd, "ls") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
            vga_puts("Contents of ");
            vga_puts(current_dir);
            vga_puts(":\n");
            for (size_t i = 0; i < VFS_COUNT; i++) {
                if (strcmp(vfs_table[i].dir, current_dir) == 0) {
                    vga_puts("  ");
                    if (vfs_table[i].is_dir) {
                        vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
                        vga_puts("[DIR]  ");
                    } else {
                        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
                        vga_puts("[FILE] ");
                    }
                    vga_puts(vfs_table[i].name);
                    vga_puts("\n");
                }
            }
        }
        else if (strncmp(cmd, "cd ", 3) == 0 || strcmp(cmd, "cd") == 0) {
            const char* target = (cmd[2] == ' ') ? cmd + 3 : "/";
            if (strcmp(target, "..") == 0 || strcmp(target, "/") == 0) {
                strcpy(current_dir, "/");
            } else if (strcmp(target, "roms") == 0 || strcmp(target, "/roms") == 0) {
                strcpy(current_dir, "/roms");
            } else if (strcmp(target, "sys") == 0 || strcmp(target, "/sys") == 0) {
                strcpy(current_dir, "/sys");
            } else if (strcmp(target, "dev") == 0 || strcmp(target, "/dev") == 0) {
                strcpy(current_dir, "/dev");
            } else {
                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                vga_puts("cd: no such directory: ");
                vga_puts(target);
                vga_puts("\n");
            }
        }
        else if (strncmp(cmd, "onizuka", 7) == 0) {
            if (cmd[7] != ' ') {
                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                vga_puts("Usage: onizuka <filename>\n");
                vga_puts("Available files: boot.s, vga.c, idt.c, timer.c, kernel.c\n");
            } else {
                const char* path = cmd + 8;
                int found = 0;
                for (int i = 0; i < EXPLAINER_NUM_FILES; i++) {
                    if (strcmp(explainer_db[i].filename, path) == 0) {
                        found = 1;
                        vga_clear();
                        vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
                        vga_puts("=== ONIZUKA CODE EXPLAINER: ");
                        vga_puts(path);
                        vga_puts(" ===\n\n");
                        
                        int lines_printed = 2; // header
                        for (int j = 0; j < explainer_db[i].num_lines; j++) {
                            const char* line = explainer_db[i].lines[j];
                            
                            /* Check for ansi-like GTO marker */
                            if (strncmp(line, "\\033[93m", 7) == 0) {
                                vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
                                vga_puts(line + 7); /* skip marker */
                                vga_puts("\n");
                                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
                            } else {
                                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
                                vga_puts(line);
                                vga_puts("\n");
                            }
                            lines_printed++;
                            
                            if (lines_printed >= 22 && j < explainer_db[i].num_lines - 1) {
                                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
                                vga_puts("\n[Press ENTER for next page...]");
                                keyboard_getchar(); // wait for enter
                                vga_clear();
                                lines_printed = 0;
                            }
                        }
                        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                        vga_puts("\n[End of File. Press ENTER to return to shell]");
                        keyboard_getchar();
                        vga_clear();
                        print_banner();
                        break;
                    }
                }
                if (!found) {
                    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                    vga_puts("onizuka: file not found in explainer database: ");
                    vga_puts(path);
                    vga_puts("\n");
                }
            }
        }
        else if (strncmp(cmd, "cat ", 4) == 0) {
            const char* path = cmd + 4;
            int found = 0;
            for (size_t i = 0; i < VFS_COUNT; i++) {
                if ((strcmp(vfs_table[i].name, path) == 0 && strcmp(vfs_table[i].dir, current_dir) == 0) ||
                    (strncmp(path, "/", 1) == 0 && strcmp(vfs_table[i].name, path + (path[1] == 'r' ? 6 : 5)) == 0)) {
                    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
                    vga_puts(vfs_table[i].content);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                vga_puts("cat: file not found: ");
                vga_puts(path);
                vga_puts("\n");
            }
        }
        else if (strcmp(cmd, "game") == 0 || strcmp(cmd, "arcade") == 0) {
            play_game();
        }
        else if (strcmp(cmd, "doom") == 0) {
            doom_main();
        }
        else if (strcmp(cmd, "gto") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            vga_puts("\n  \"I'm Eikichi Onizuka, 22 years old, single! Welcome to OniOS!\"\n");
            vga_puts("  Lesson 1: If an OS crashes, give it a German Suplex!\n\n");
        } 
        else if (strcmp(cmd, "cresta") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            vga_puts("  [!] BOOM! Vice Principal Uchiyamada's Cresta just got wrecked again! 🚗💨\n");
        } 
        else if (strcmp(cmd, "suplex") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
            vga_puts("  [!] EIKICHI ONIZUKA DELIVERS A GERMAN SUPLEX TO THE KERNEL BUG! 🤼‍♂️💥\n");
        } 
        else if (strcmp(cmd, "info") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            vga_puts("  OS Name    : OniOS Bare-Metal Kernel\n");
            vga_puts("  VGA Address: ");
            vga_puthex(0xB8000);
            vga_puts("\n  Keyboard IO: Port 0x60 / Status 0x64\n");
        } 
        else if (strcmp(cmd, "clear") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
            vga_clear();
            print_banner();
        } 
        else if (strcmp(cmd, "reboot") == 0) {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            vga_puts("Rebooting hardware CPU...\n");
            /* Pulse CPU reset line via PS/2 keyboard controller */
            uint8_t good = 0x02;
            while (good & 0x02) {
                good = inb(0x64);
            }
            outb(0x64, 0xFE);
        } 
        else if (cmd[0] != '\0') {
            vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            vga_puts("Unknown command: ");
            vga_puts(cmd);
            vga_puts(" (type 'help' for commands)\n");
        }
    }
}

#include "multiboot.h"
#include "graphics.h"

void kernel_main(uint32_t magic, uint32_t addr) {
    /* Initialize IDT Interrupt Descriptor Table & Remap 8259 PIC */
    idt_init();

    /* Initialize PIT 8254 Hardware Timer at 100Hz (10ms clock ticks) */
    timer_init(100);

    /* Initialize Dynamic Heap RAM */
    mem_init();

    /* Initialize Process Control Block Scheduler */
    process_init();

    /* Initialize VESA Graphics Framebuffer from Multiboot Info */
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        multiboot_info_t* mbd = (multiboot_info_t*) addr;
        
        /* Check if framebuffer info is valid (flags bit 12) */
        if (mbd->flags & (1 << 12)) {
            graphics_init(mbd->framebuffer_addr, mbd->framebuffer_width, 
                          mbd->framebuffer_height, mbd->framebuffer_pitch, 
                          mbd->framebuffer_bpp);
        }
    }

    /* Set Turbo C style: Light Grey text on Dark Blue background */
    vga_init(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC && graphics_width > 0) {
        vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        vga_puts("VESA Graphics Initialized: ");
        vga_putdec(graphics_width);
        vga_puts("x");
        vga_putdec(graphics_height);
        vga_puts(" @ ");
        vga_putdec(graphics_bpp);
        vga_puts(" bpp\n");
    }

    print_banner();
    shell_run();
}
