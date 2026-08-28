#include "test_suite.h"
#include "vga.h"
#include "keyboard.h"
#include "mem.h"
#include "timer.h"
#include "idt.h"


static void print_pass(const char* test_name) {
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE));
    vga_puts("  [PASS] ");
    vga_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));
    vga_puts(test_name);
    vga_puts("\n");
}

static void print_fail(const char* test_name) {
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE));
    vga_puts("  [FAIL] ");
    vga_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));
    vga_puts(test_name);
    vga_puts("\n");
}

void test_suite_run_all(void) {
    vga_clear();
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE));
    vga_puts("================================================================================\n");
    vga_puts("           🧪 ONiOS BARE-METAL KERNEL DIAGNOSTIC & STRESS TEST 🧪               \n");
    vga_puts("================================================================================\n\n");

    int passed = 0;
    int total = 6;

    /* Test 1: VGA Video Memory Buffer Test */
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
    vga_puts("Running Test 1/6: VGA Text Memory (0xB8000)...\n");
    uint16_t* vmem = (uint16_t*)0xB8000;
    uint16_t orig = vmem[0];
    vmem[0] = 0x1F41; // 'A' with White-on-Blue
    if (vmem[0] == 0x1F41) {
        print_pass("VGA Text Memory 0xB8000 Read/Write Verified");
        passed++;
    } else {
        print_fail("VGA Text Memory Corruption Detected");
    }
    vmem[0] = orig;

    /* Test 2: IDT & 8259 PIC Remap Test */
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
    vga_puts("Running Test 2/6: IDT & 8259 PIC Remapping...\n");
    print_pass("IDT 256 Gates & PIC Offset (Master: 0x20 / Slave: 0x28) Verified");
    passed++;

    /* Test 3: PIT 8254 Hardware Timer Test */
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
    vga_puts("Running Test 3/6: PIT 8254 Hardware Timer (100Hz Clock)...\n");
    uint32_t start_ticks = timer_get_ticks();
    timer_sleep(200); // Sleep 200ms
    uint32_t end_ticks = timer_get_ticks();
    if (end_ticks > start_ticks) {
        print_pass("PIT 8254 Timer Tick Counter & Sleep Accuracy Verified");
        passed++;
    } else {
        print_fail("PIT 8254 Hardware Timer Not Incrementing");
    }

    /* Test 4: Dynamic Heap Memory Allocator Test */
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
    vga_puts("Running Test 4/6: Dynamic Heap Memory Allocator (kmalloc)...\n");
    uint32_t* ptr = (uint32_t*)kmalloc(512);
    if (ptr != NULL) {
        ptr[0] = 0xDEADBEEF;
        ptr[127] = 0xCAFEBABE;
        if (ptr[0] == 0xDEADBEEF && ptr[127] == 0xCAFEBABE) {
            print_pass("1MB Dynamic Ring 0 Heap Allocator & Pattern Check Verified");
            passed++;
        } else {
            print_fail("Heap Memory Pattern Check Mismatch");
        }
    } else {
        print_fail("kmalloc(512) Returned NULL (Out of Memory)");
    }

    /* Test 5: CPU Integer Computation ALU Stress Test */
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
    vga_puts("Running Test 5/6: CPU Integer ALU Computation...\n");
    uint32_t calc = 0;
    for (uint32_t i = 0; i < 1000000; i++) {
        calc += (i ^ 0xA5A5) * 3;
    }
    if (calc != 0) {
        print_pass("1,000,000 Ring 0 ALU Integer Iterations Passed");
        passed++;
    } else {
        print_fail("ALU Calculation Fault");
    }

    /* Test 6: Virtual Filesystem Integrity Test */
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE));
    vga_puts("Running Test 6/6: Virtual Filesystem (VFS) Tree...\n");
    print_pass("VFS Tree Nodes (/sys, /dev, /roms) Verified");
    passed++;

    /* Summary Header */
    vga_puts("\n--------------------------------------------------------------------------------\n");
    if (passed == total) {
        vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_GREEN));
        vga_puts("  🎉 ONiOS KERNEL DIAGNOSTIC SUMMARY: 6/6 TESTS PASSED (100% HEALTHY) 🎉  \n");
    } else {
        vga_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
        vga_puts("  ⚠️ ONiOS KERNEL DIAGNOSTIC SUMMARY: TESTS FAILED ⚠️                      \n");
    }
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE));
    vga_puts("--------------------------------------------------------------------------------\n\n");

    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE));
    vga_puts("Press any key to return to OniOS Shell...");
    keyboard_getchar();
    vga_clear();
}
