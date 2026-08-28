import struct

wad_path = '/home/aizen-sosuke/iso/OniOS/doom_files/DOOMS/DOOM1.WAD'
out_h = '/home/aizen-sosuke/iso/OniOS/e1m1_data.h'

with open(wad_path, 'rb') as f:
    header = f.read(12)
    sig, num_lumps, dir_pos = struct.unpack('<4sII', header)
    f.seek(dir_pos)
    
    lumps = {}
    for i in range(num_lumps):
        pos, size, name = struct.unpack('<II8s', f.read(16))
        clean_name = name.decode('ascii', errors='ignore').split('\x00')[0].strip()
        lumps[i] = (clean_name, pos, size)

    e1m1_idx = None
    for idx, (name, pos, size) in lumps.items():
        if name == 'E1M1':
            e1m1_idx = idx
            break

    if e1m1_idx is not None:
        things_pos, things_size = lumps[e1m1_idx + 1][1], lumps[e1m1_idx + 1][2]
        linedefs_pos, linedefs_size = lumps[e1m1_idx + 2][1], lumps[e1m1_idx + 2][2]
        vertexes_pos, vertexes_size = lumps[e1m1_idx + 4][1], lumps[e1m1_idx + 4][2]

        # Read Vertexes (short x, short y)
        f.seek(vertexes_pos)
        num_vertexes = vertexes_size // 4
        verts = []
        for _ in range(num_vertexes):
            vx, vy = struct.unpack('<hh', f.read(4))
            verts.append((vx, vy))

        # Read Linedefs (short v1, short v2, short flags, short special, short tag, short sidenum[2])
        f.seek(linedefs_pos)
        num_linedefs = linedefs_size // 14
        lines = []
        for _ in range(num_linedefs):
            v1, v2, flags, special, tag, s1, s2 = struct.unpack('<hhhhhhh', f.read(14))
            lines.append((v1, v2, flags))

        # Read Things (short x, short y, short angle, short type, short flags)
        f.seek(things_pos)
        num_things = things_size // 10
        things = []
        for _ in range(num_things):
            tx, ty, angle, ttype, tflags = struct.unpack('<hhhhh', f.read(10))
            things.append((tx, ty, ttype))

        with open(out_h, 'w') as out:
            out.write('/* Real 1993 DOOM1.WAD E1M1 Level Geometry */\n')
            out.write('#ifndef E1M1_DATA_H\n#define E1M1_DATA_H\n\n')
            out.write('#include <stdint.h>\n\n')
            
            out.write(f'#define NUM_E1M1_VERTS {num_vertexes}\n')
            out.write(f'#define NUM_E1M1_LINES {num_linedefs}\n')
            out.write(f'#define NUM_E1M1_THINGS {num_things}\n\n')

            out.write('typedef struct { int16_t x, y; } doom_vert_t;\n')
            out.write('typedef struct { int16_t v1, v2; uint16_t flags; } doom_line_t;\n')
            out.write('typedef struct { int16_t x, y; int16_t type; } doom_thing_t;\n\n')

            out.write('static const doom_vert_t e1m1_verts[] = {\n')
            for v in verts:
                out.write(f'    {{{v[0]}, {v[1]}}},\n')
            out.write('};\n\n')

            out.write('static const doom_line_t e1m1_lines[] = {\n')
            for l in lines:
                out.write(f'    {{{l[0]}, {l[1]}, 0x{l[2]:04x}}},\n')
            out.write('};\n\n')

            out.write('static const doom_thing_t e1m1_things[] = {\n')
            for t in things:
                out.write(f'    {{{t[0]}, {t[1]}, {t[2]}}},\n')
            out.write('};\n\n#endif\n')

        print(f"Successfully extracted E1M1: {num_vertexes} Vertices, {num_linedefs} Linedefs, {num_things} Things!")
    else:
        print("E1M1 not found.")
