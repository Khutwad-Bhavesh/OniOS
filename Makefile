# OniOS Bare-Metal Kernel Makefile

CC = gcc
AS = gcc
LD = ld
RUSTC = rustc

# Check if rustc with i686 target support is available
HAS_RUST := $(shell rustc --target i686-unknown-linux-gnu --emit=obj /dev/null -o /dev/null 2>/dev/null && echo 1)

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -std=gnu99 -nostdlib -fno-builtin -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mpreferred-stack-boundary=2
ASFLAGS = -m32 -c
RUSTFLAGS = --target i686-unknown-linux-gnu --emit=obj -O -C panic=abort
LDFLAGS = -m elf_i386 -T linker.ld

ifneq ($(HAS_RUST),)
CFLAGS += -DHAS_RUST
OBJS = boot.o vga.o vga13.o keyboard.o doom_engine.o kernel.o safety.o
else
OBJS = boot.o vga.o vga13.o keyboard.o doom_engine.o kernel.o
endif

TARGET = kernel.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

boot.o: boot.s
	$(AS) $(ASFLAGS) $< -o $@

vga.o: vga.c vga.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

vga13.o: vga13.c vga13.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

keyboard.o: keyboard.c keyboard.h io.h vga.h
	$(CC) $(CFLAGS) -c $< -o $@

doom_engine.o: doom_engine.c doom_engine.h vga.h vga13.h keyboard.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.c vga.h keyboard.h io.h doom_engine.h
	$(CC) $(CFLAGS) -c $< -o $@



safety.o: safety.rs
	$(RUSTC) $(RUSTFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)

.PHONY: all clean run
