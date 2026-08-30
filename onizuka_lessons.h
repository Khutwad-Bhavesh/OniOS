#ifndef ONIZUKA_LESSONS_H
#define ONIZUKA_LESSONS_H

struct onizuka_step {
    const char* text;
};

struct onizuka_lesson {
    const char* topic;
    const char* title;
    const char** diagram;
    int diagram_lines;
    const struct onizuka_step* steps;
    int num_steps;
};

/* --- LESSON 1: BOOT --- */
static const char* boot_diagram[] = {
    "  [ BIOS/UEFI ]  -->  [ GRUB Bootloader ]  -->  [ OniOS Multiboot Header ]",
    "         |                    |                         |",
    "    (Hardware)        (Finds kernel on disk)    (Validates Magic 0x1BADB002)",
    "                                                        |",
    "                                                        V",
    "                                                 [ kernel_main() ]"
};

static const struct onizuka_step boot_steps[] = {
    {"Step 1: When you power on the PC, the BIOS runs hardware checks and loads the bootloader (GRUB)."},
    {"Step 2: GRUB searches our disk for the OniOS kernel binary. It looks for a special 32-bit \\033[93m\"magic number\" (0x1BADB002)\\033[0m in the first 8KB."},
    {"Step 3: If the magic number matches, GRUB loads the kernel into memory, sets up a 32-bit protected mode environment..."},
    {"Step 4: Finally, GRUB jumps to our \\033[93m_start\\033[0m assembly label, which sets up the stack and calls \\033[93mkernel_main()\\033[0m!"}
};

/* --- LESSON 2: INTERRUPTS --- */
static const char* int_diagram[] = {
    "  [ Hardware Device ] ---> [ PIC 8259 ] ---> [ CPU INTR Pin ]",
    "     (e.g., PIT timer)        (IRQ 0)               |",
    "                                                    |",
    "                                                    V",
    "                             [ IDT (Interrupt Descriptor Table) ]",
    "                             | Gate 32 points to our irq0.s     |",
    "                             +----------------------------------+",
    "                                                    |",
    "                                                    V",
    "                                           [ C timer_handler() ]"
};

static const struct onizuka_step int_steps[] = {
    {"Step 1: A hardware device like the PIT (Programmable Interval Timer) fires an electrical signal."},
    {"Step 2: The Programmable Interrupt Controller (PIC) receives this signal (IRQ0), checks if it's masked, and signals the CPU."},
    {"Step 3: The CPU pauses current execution, looks up the interrupt vector (32) in the \\033[93mIDT\\033[0m, and jumps to our Assembly wrapper!"},
    {"Step 4: The wrapper uses \\033[93mpushal\\033[0m to save registers, calls our C handler to tick time, and acknowledges the PIC."},
    {"Step 5: The wrapper calls \\033[93mpopal\\033[0m and \\033[93miret\\033[0m to restore registers and seamlessly resume the game loop."}
};

/* --- LESSON 3: VGA --- */
static const char* vga_diagram[] = {
    " Memory Address: 0xB8000",
    " +----------------+----------------+----------------+",
    " | CHAR: 'O' (79) | ATTR: 0x0F     | CHAR: 'n' (110)| ...",
    " +----------------+----------------+----------------+",
    "  (White text, Black bg)",
    "",
    " Formula: \\033[93moffset = (y * 80 + x) * 2;\\033[0m"
};

static const struct onizuka_step vga_steps[] = {
    {"Step 1: VGA Text Mode maps the screen to a specific region of RAM starting at \\033[93m0xB8000\\033[0m."},
    {"Step 2: Every character on the screen takes exactly 2 bytes in memory: one for the ASCII character code, and one for the color attribute."},
    {"Step 3: The screen is 80 columns wide and 25 rows high. To print a character at (x, y), we write to: 0xB8000 + (y * 80 + x) * 2."}
};

/* --- LESSON DB --- */
static const struct onizuka_lesson onizuka_db[] = {
    {
        .topic = "boot",
        .title = "Lesson 1: The Boot Process & Multiboot",
        .diagram = boot_diagram,
        .diagram_lines = 6,
        .steps = boot_steps,
        .num_steps = 4
    },
    {
        .topic = "interrupts",
        .title = "Lesson 2: Hardware Interrupts & The IDT",
        .diagram = int_diagram,
        .diagram_lines = 10,
        .steps = int_steps,
        .num_steps = 5
    },
    {
        .topic = "vga",
        .title = "Lesson 3: VGA Text Mode Memory",
        .diagram = vga_diagram,
        .diagram_lines = 7,
        .steps = vga_steps,
        .num_steps = 3
    }
};

#define ONIZUKA_NUM_LESSONS 3

#endif
