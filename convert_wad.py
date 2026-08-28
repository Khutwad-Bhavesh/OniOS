import struct

wad_path = '/home/aizen-sosuke/iso/OniOS/doom_files/DOOMS/DOOM1.WAD'
out_h = '/home/aizen-sosuke/iso/OniOS/endoom.h'

with open(wad_path, 'rb') as f:
    header = f.read(12)
    sig, num_lumps, dir_pos = struct.unpack('<4sII', header)
    f.seek(dir_pos)
    
    endoom_pos = None
    endoom_size = 0
    
    for i in range(num_lumps):
        pos, size, name = struct.unpack('<II8s', f.read(16))
        lump_name = name.decode('ascii', errors='ignore').split('\x00')[0].strip()
        if lump_name == 'ENDOOM':
            endoom_pos = pos
            endoom_size = size
            break

    if endoom_pos is not None:
        f.seek(endoom_pos)
        data = f.read(endoom_size)
        
        with open(out_h, 'w') as out:
            out.write('/* Authentic Id Software 1993 DOOM1.WAD ENDOOM Screen */\n')
            out.write('#ifndef ENDOOM_H\n#define ENDOOM_H\n\n')
            out.write('#include <stdint.h>\n\n')
            out.write('static const uint8_t endoom_data[4000] = {\n')
            for b in range(len(data)):
                out.write(f'0x{data[b]:02x}, ')
                if (b + 1) % 12 == 0:
                    out.write('\n')
            out.write('\n};\n\n#endif\n')
        print("Successfully generated endoom.h from DOOM1.WAD!")
    else:
        print("ENDOOM lump not found.")
