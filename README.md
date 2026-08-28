# ⚡ OniOS: Bare-Metal Unix-like Kernel & Retro Arcade Engine

> **"Can it run DOOM? YES, OniOS runs 3D DOOM natively on bare-metal!"**

![Architecture](https://img.shields.io/badge/Architecture-x86%2032--bit%20Multiboot-blue)
![Languages](https://img.shields.io/badge/Languages-C%20%7C%20Assembly%20%7C%20Rust-brightgreen)
![Aesthetics](https://img.shields.io/badge/Aesthetics-Turbo%20C%20Yellow--on--Blue-gold)
![License](https://img.shields.io/badge/License-GPLv3-green)

**OniOS** is a standalone, 32-bit Multiboot Unix-like kernel written from scratch in **C**, **x86 Assembly**, and **Rust (`no_std`)**. Built with zero POSIX or Linux kernel dependencies, OniOS features direct hardware memory access (`0xB8000`), PS/2 keyboard scancode drivers, virtual file system traversal (`cd`, `ls`, `cat`), a built-in VGA arcade game (**Onizuka Cresta Dodge**), and a native **3D DOOM Raycasting Engine**!

Built by **Bhavesh Khutwad**.

---

## ✨ Key Features

- 🚀 **Zero Dependencies**: Boots directly via Multiboot (`0x1BADB002`) in QEMU or GRUB as an independent OS kernel.
- 🎨 **Turbo C Aesthetic**: Direct 80x25 VGA text memory driver (`0xB8000`) with classic yellow-on-blue theme, hardware cursor, and smooth scrolling.
- ⌨️ **PS/2 Keyboard Hardware Driver**: Reads raw CPU scancodes from port `0x60` for interactive shell typing.
- 📂 **Virtual Filesystem & Traversal**:
  - `cd <dir>`: Navigate subdirectories (`/roms`, `/sys`, `/dev`, `/`).
  - `ls`: List directory files and subfolders.
  - `cat <file>`: Display file contents (including `doom1.wad` WAD header).
  - `pwd`: Print current working directory.
- 🎮 **Bare-Metal Onizuka Cresta Dodge Game (`game`)**: Interactive VGA arcade action game with collision detection & score tracking.
- 💥 **Native 3D DOOM Raycasting Engine (`doom`)**:
  - Standalone DDA 3D raycasting renderer.
  - Real-time DOOM HUD (`HEALTH`, `AMMO`, `KILLS`, `WEAPON`).
  - Interactive movement (`W`/`A`/`S`/`D`) and shotgun controls (`Space`/`F`).
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
  doom     - Launch 3D Bare-Metal DOOM Raycasting Engine!
  cd <dir> - Change working directory (e.g. 'cd roms', 'cd sys', 'cd ..')
  ls       - List directory contents & subfolders
  cat <f>  - Display virtual file content
  pwd      - Print current directory
  game     - Launch Onizuka Cresta Dodge Bare-Metal Arcade!
  gto      - Display GTO ASCII quote & lesson
  cresta   - Vice Principal Uchiyamada's Cresta status
  suplex   - Execute German Suplex on system bugs
  info     - Display hardware & memory info
  clear    - Clear screen buffer
  reboot   - Hardware CPU reboot via keyboard controller
```

---

## 📜 License & Credits

- **License**: GNU General Public License v3.0 (GPLv3)
- **Author**: Bhavesh Khutwad
- Dedicated with respect to *Great Teacher Onizuka*! 👊🏍️🔥
