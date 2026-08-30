.global irq0_handler_asm
.extern timer_handler

.section .text
.align 4
irq0_handler_asm:
    pushal
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    movw $0x18, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    cld
    call timer_handler

    popl %gs
    popl %fs
    popl %es
    popl %ds
    popal
    iret
