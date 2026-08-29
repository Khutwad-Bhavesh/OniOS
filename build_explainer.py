import os

FILES_TO_EXPLAIN = [
    "boot.s",
    "vga.c",
    "idt.c",
    "timer.c",
    "kernel.c"
]

# GTO's Teaching Notes mapped to keywords/regex
EXPLANATIONS = {
    "0xB8000": "GTO Lesson: 0xB8000 is the hardcoded physical memory address for the VGA Text Buffer! We write bytes here, and the graphics card instantly draws them on screen!",
    "outb(0x20, 0x11)": "GTO Lesson: We are talking directly to the master PIC (Programmable Interrupt Controller) hardware chip on port 0x20. We must remap it so hardware interrupts don't conflict with CPU faults!",
    "1193182": "GTO Lesson: 1193182 Hz is the exact hardware crystal oscillator frequency of the PIT 8254 chip on every x86 motherboard! We divide this to set our clock speed.",
    "MULTIBOOT_PAGE_ALIGN": "GTO Lesson: Multiboot Magic! This header tells the GRUB bootloader 'Hey, I am a valid OS kernel! Load me into RAM!'",
    "__asm__ __volatile__": "GTO Lesson: Inline Assembly! We are bypassing C and writing raw machine instructions straight to the CPU!",
    "process_yield": "GTO Lesson: Cooperative Multitasking! We save the CPU state (ESP register) and switch to the next process's stack!",
    "0x60": "GTO Lesson: I/O Port 0x60 is the PS/2 Keyboard Data Port! Every time you press a key, the hardware sends the scancode here!",
}

def generate_header():
    out = "#ifndef EXPLAINER_DATA_H\n#define EXPLAINER_DATA_H\n\n"
    out += "struct explained_file {\n    const char* filename;\n    const char** lines;\n    int num_lines;\n};\n\n"
    
    structs = []
    
    for idx, filename in enumerate(FILES_TO_EXPLAIN):
        if not os.path.exists(filename):
            continue
            
        with open(filename, "r") as f:
            lines = f.readlines()
            
        out += f"static const char* file_{idx}_lines[] = {{\n"
        
        line_count = 0
        for line in lines:
            line = line.replace('\n', '').replace('\r', '')
            # Escape quotes and backslashes
            line = line.replace('\\', '\\\\').replace('"', '\\"')
            out += f"    \"{line}\",\n"
            line_count += 1
            
            # Check for GTO explanations
            for key, exp in EXPLANATIONS.items():
                if key in line:
                    exp_escaped = exp.replace('"', '\\"')
                    out += f"    \"\\033[93m=> {exp_escaped}\\033[0m\",\n" # We can use an ansi-like marker or just plain text
                    line_count += 1
                    
        out += "};\n\n"
        structs.append((filename, f"file_{idx}_lines", line_count))
        
    out += f"static const struct explained_file explainer_db[{len(structs)}] = {{\n"
    for s in structs:
        out += f"    {{\"{s[0]}\", {s[1]}, {s[2]}}},\n"
    out += "};\n\n"
    
    out += f"static const int EXPLAINER_NUM_FILES = {len(structs)};\n\n"
    out += "#endif\n"
    
    with open("explainer_data.h", "w") as f:
        f.write(out)

if __name__ == "__main__":
    generate_header()
