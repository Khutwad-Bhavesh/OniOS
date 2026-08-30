# OniOS Bare-Metal Kernel Makefile
CC = gcc
LD = ld
AS = gcc
RUSTC = rustc

CFLAGS = -m32 -ffreestanding -O0 -g -Wall -Wextra -std=gnu99 -nostdlib -fno-builtin -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mpreferred-stack-boundary=2 -Ilibc -Idoom -DDOOMGENERIC
LDFLAGS = -m elf_i386 -T linker.ld
ASFLAGS = -m32 -c
RUSTFLAGS = --target i686-unknown-linux-gnu --emit=obj -C panic=abort

HAS_RUST := $(shell rustc --target i686-unknown-linux-gnu --emit=obj /dev/null -o /dev/null 2>/dev/null && echo 1)

DOOM_SRCS = $(wildcard doom/*.c)
DOOM_OBJS = $(DOOM_SRCS:.c=.o)

ifneq ($(HAS_RUST),)
CFLAGS += -DHAS_RUST
OBJS = boot.o irq0.o vga.o keyboard.o idt.o timer.o mem.o process.o graphics.o editor.o doom_engine.o audio.o test_suite.o kernel.o safety.o libc_stubs.o $(DOOM_OBJS)
else
OBJS = boot.o irq0.o vga.o keyboard.o idt.o timer.o mem.o process.o graphics.o editor.o doom_engine.o audio.o test_suite.o kernel.o libc_stubs.o $(DOOM_OBJS)
endif

TARGET = kernel.bin
ISO_TARGET = OniOS.iso

LIBGCC := $(shell $(CC) -m32 -print-libgcc-file-name)

all: $(ISO_TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBGCC)

$(ISO_TARGET): $(TARGET) doom1.wad
	mkdir -p isodir/boot/grub
	cp $(TARGET) isodir/boot/$(TARGET)
	cp doom1.wad isodir/boot/doom1.wad
	echo 'set timeout=0' > isodir/boot/grub/grub.cfg
	echo 'set default=0' >> isodir/boot/grub/grub.cfg
	echo 'menuentry "OniOS" {' >> isodir/boot/grub/grub.cfg
	echo '  set gfxpayload=800x600x32' >> isodir/boot/grub/grub.cfg
	echo '  multiboot /boot/$(TARGET)' >> isodir/boot/grub/grub.cfg
	echo '  module /boot/doom1.wad' >> isodir/boot/grub/grub.cfg
	echo '  boot' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_TARGET) isodir

boot.o: boot.s
	$(CC) -m32 -c $< -o $@

vga.o: vga.c vga.h graphics.h
	$(CC) $(CFLAGS) -c $< -o $@

keyboard.o: keyboard.c keyboard.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

idt.o: idt.c idt.h io.h keyboard.h timer.h
	$(CC) $(CFLAGS) -c $< -o $@

timer.o: timer.c timer.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

mem.o: mem.c mem.h
	$(CC) $(CFLAGS) -c $< -o $@

graphics.o: graphics.c graphics.h font8x8.h
	$(CC) $(CFLAGS) -c $< -o $@

process.o: process.c process.h mem.h vga.h
	$(CC) $(CFLAGS) -c $< -o $@

editor.o: editor.c editor.h vga.h keyboard.h
	$(CC) $(CFLAGS) -c $< -o $@

test_suite.o: test_suite.c test_suite.h vga.h mem.h timer.h process.h graphics.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.c vga.h idt.h keyboard.h mem.h process.h editor.h test_suite.h multiboot.h graphics.h
	python3 build_explainer.py
	$(CC) $(CFLAGS) -c $< -o $@

safety.o: safety.rs
	$(RUSTC) $(RUSTFLAGS) $< -o $@

clean:
	rm -f *.o doom/*.o $(TARGET) $(ISO_TARGET)
	rm -rf isodir

run: $(ISO_TARGET)
	qemu-system-i386 -cdrom $(ISO_TARGET) -vga std
