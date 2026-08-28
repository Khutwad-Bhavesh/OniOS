# OniOS Bare-Metal Kernel Makefile
CC = gcc
LD = ld
AS = gcc
RUSTC = rustc

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -std=gnu99 -nostdlib -fno-builtin -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mpreferred-stack-boundary=2
LDFLAGS = -m elf_i386 -T linker.ld
ASFLAGS = -m32 -c
RUSTFLAGS = --target i686-unknown-linux-gnu --emit=obj -C panic=abort

HAS_RUST := $(shell rustc --target i686-unknown-linux-gnu --emit=obj /dev/null -o /dev/null 2>/dev/null && echo 1)

ifneq ($(HAS_RUST),)
ifneq ($(HAS_RUST),)
CFLAGS += -DHAS_RUST
OBJS = boot.o vga.o keyboard.o idt.o timer.o mem.o editor.o kernel.o safety.o
else
OBJS = boot.o vga.o keyboard.o idt.o timer.o mem.o editor.o kernel.o
endif

TARGET = kernel.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

boot.o: boot.s
	$(AS) $(ASFLAGS) $< -o $@

vga.o: vga.c vga.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

keyboard.o: keyboard.c keyboard.h io.h vga.h
	$(CC) $(CFLAGS) -c $< -o $@

idt.o: idt.c idt.h io.h vga.h
	$(CC) $(CFLAGS) -c $< -o $@

timer.o: timer.c timer.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

mem.o: mem.c mem.h
	$(CC) $(CFLAGS) -c $< -o $@

editor.o: editor.c editor.h vga.h keyboard.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.c vga.h keyboard.h io.h idt.h timer.h mem.h editor.h
	$(CC) $(CFLAGS) -c $< -o $@


safety.o: safety.rs
	$(RUSTC) $(RUSTFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)
