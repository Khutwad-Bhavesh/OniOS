# ⚡ OniOS: The Ultimate Educational Bare-Metal OS

> **"Built for students. Built for learning. And yes... it runs DOOM on bare-metal!"**

![Architecture](https://img.shields.io/badge/Architecture-x86%2032--bit%20Multiboot-blue)
![Languages](https://img.shields.io/badge/Languages-C%20%7C%20Assembly%20%7C%20Rust-brightgreen)
![Educational](https://img.shields.io/badge/Purpose-Educational%20OS%20Workshop-gold)
![License](https://img.shields.io/badge/License-GPLv3-green)

**OniOS** is a standalone, 32-bit Multiboot Unix-like kernel written from scratch in **C**, **x86 Assembly**, and **Rust (`no_std`)**. 

Designed specifically as a teaching platform for a **7-Day OS Building Workshop**, OniOS strips away the magic of modern operating systems to show students exactly how a computer works under the hood. Built with zero POSIX or Linux kernel dependencies, it features direct hardware memory access, custom hardware drivers, and interactive built-in lessons to make learning systems programming incredibly fun and rewarding!

Built by **Bhavesh Khutwad**.

---

## ✨ Key Features

- 🚀 **Zero Dependencies**: Boots directly via Multiboot (`0x1BADB002`) in QEMU or GRUB as an independent OS kernel.
- 🎨 **Turbo C Aesthetic**: Direct 80x25 VGA text memory driver (`0xB8000`) with classic yellow-on-blue theme, hardware cursor, and smooth scrolling.
- 🏫 **"Great Teacher Onizuka" Explainer Lessons**: Built-in interactive ASCII lessons to teach students about Booting, Interrupts, VGA, and the Kernel (`onizuka` command).
- 🔊 **PC Speaker Audio (`play`)**: Native PIT-driven PC Speaker sequencer that plays the DOOM E1M1 theme directly from the motherboard.
- 🖱️ **Hardware Drivers**: Custom PS/2 Keyboard and PS/2 Mouse drivers, mapped through the IDT and PICs.
- 📂 **Virtual Filesystem & Traversal**:
  - `cd <dir>`: Navigate subdirectories (`/roms`, `/sys`, `/dev`, `/`).
  - `ls`: List directory files and subfolders.
  - `cat <file>`: Display file contents (including `doom1.wad` WAD header).
  - `pwd`: Print current working directory.
- 🎮 **Bare-Metal Onizuka Cresta Dodge Game (`game`)**: Interactive VGA arcade action game with collision detection & score tracking.
- 💥 **Native 3D DOOM (id Software) Port (`doom`)**:
  - Full integration of the *Doomgeneric* port into the bare-metal kernel.
  - VESA 32-bit linear framebuffer rendering (Graphics Mode).
  - Uses the actual `doom1.wad` loaded via GRUB multiboot modules!
  - Fully working PS/2 Keyboard handling for in-game controls.
- 🛡️ **Rust Memory Safety**: Integrated `#![no_std]` Rust subsystem module exporting safety verification checks.

---

## 🛠️ How to Compile & Run OniOS

### Prerequisites
- `gcc` (with 32-bit support `-m32`)
- `make`
- `qemu-system-i386`
- *(Optional)* `rustc` for Rust memory safety module.

### Commands

```bash
# Clone repository
git clone https://github.com/Khutwad-Bhavesh/OniOS.git
cd OniOS

# Compile & Launch directly in QEMU
make clean
make run
```

---

## 🎮 Interactive Commands inside OniOS Shell

```text
OniOS:/> help

Available OniOS Commands:
  doom     - Launch id Software DOOM (Doomgeneric port)
  play     - Play the DOOM E1M1 Theme via PC Speaker
  onizuka  - Run Great Teacher Onizuka's Interactive Lessons (e.g., 'onizuka kernel')
  cd <dir> - Change working directory (e.g., 'cd roms', 'cd sys', 'cd ..')
  ls       - List directory contents & subfolders
  cat <f>  - Display virtual file content
  pwd      - Print current directory
  game     - Launch Onizuka Cresta Dodge Bare-Metal Arcade!
  gto      - Display GTO ASCII quote
  cresta   - Vice Principal Uchiyamada's Cresta status
  suplex   - Execute German Suplex on system bugs
  info     - Display hardware & memory info
  clear    - Clear screen buffer
  ps       - List active processes & threads
  fork     - Spawn a background test process
  yield    - Yield CPU to next process
  reboot   - Hardware CPU reboot via keyboard controller
```

---

## 📜 License & Credits

- **License**: GNU General Public License v3.0 (GPLv3)
- **Author**: Bhavesh Khutwad
- Dedicated with respect to *Great Teacher Onizuka*! 👊🏍️🔥
