/*
 * OniOS - Bare-Metal Kernel Bootstrapper
 * Multiboot 1 Compliant Header & Entry
 */

.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set VIDMODE,  1<<2             /* request video mode */
.set FLAGS,    ALIGN | MEMINFO | VIDMODE
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above to prove multiboot */

/* Multiboot header MUST be in the first 8KB of binary */
.section .multiboot,"a"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0, 0, 0, 0, 0           /* Address fields (ignored for ELF) */
.long 0                       /* mode_type: 0 for linear graphics */
.long 800                     /* width */
.long 600                     /* height */
.long 32                      /* depth (bpp) */

/* 16 KiB Stack */
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

/* Entry point */
.section .text
.global _start
.type _start, @function
_start:
    /* Disable hardware interrupts during boot */
    cli

    /* Set up stack pointer */
    mov $stack_top, %esp

    /* Reset EFLAGS register */
    pushl $0
    popf

    /* Push Multiboot info structure pointer (ebx) and magic number (eax) */
    pushl %ebx
    pushl %eax

    /* Call C kernel main function */
    call kernel_main

    /* Infinite halt loop if kernel_main ever returns */
1:  cli
    hlt
    jmp 1b

.size _start, . - _start
