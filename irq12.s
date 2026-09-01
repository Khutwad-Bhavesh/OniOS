.section .text
.global irq12_handler_asm
.type irq12_handler_asm, @function
.extern mouse_handler

irq12_handler_asm:
    pushal

    /* Load kernel data segments */
    movw $0x18, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    call mouse_handler

    /* Restore registers and return */
    popal
    iret
