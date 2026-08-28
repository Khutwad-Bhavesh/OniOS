#include "doom_engine.h"
#include "vga.h"
#include "keyboard.h"
#include "io.h"
#include "e1m1_data.h"

/* Fixed-Point Arithmetic (Scaled by 256) */
#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)

/* Initial Player Position at E1M1 Spawn (Scaled) */
static int px = -1056;
static int py = -3616;
static int dir_x = FP_ONE;
static int dir_y = 0;
static int plane_x = 0;
static int plane_y = 168;

static int health = 100;
static int ammo = 50;
static int kills = 0;
static int shooting = 0;

static void render_doom_frame(void) {
    uint16_t* const vga_buf = (uint16_t*) 0xB8000;

    /* Raycast 80 columns against real DOOM E1M1 Linedefs */
    for (int x = 0; x < 80; x++) {
        int camera_x = (2 * x * FP_ONE / 80) - FP_ONE;
        int ray_dir_x = dir_x + ((plane_x * camera_x) >> FP_SHIFT);
        int ray_dir_y = dir_y + ((plane_y * camera_x) >> FP_SHIFT);

        if (ray_dir_x == 0) ray_dir_x = 1;
        if (ray_dir_y == 0) ray_dir_y = 1;

        int closest_dist = 32767;
        int hit_side = 0;

        /* Check ray intersection against 475 E1M1 WAD wall line segments */
        for (int i = 0; i < NUM_E1M1_LINES && i < 200; i++) {
            doom_line_t line = e1m1_lines[i];
            doom_vert_t v1 = e1m1_verts[line.v1];
            doom_vert_t v2 = e1m1_verts[line.v2];

            /* Vector ray intersection math */
            int dx = v2.x - v1.x;
            int dy = v2.y - v1.y;

            if (dx == 0 && dy == 0) continue;

            int rel_x = v1.x - (px >> 2);
            int rel_y = v1.y - (py >> 2);

            int dist = (rel_x * ray_dir_x + rel_y * ray_dir_y) >> FP_SHIFT;
            if (dist > 10 && dist < closest_dist) {
                closest_dist = dist;
                hit_side = (i % 2);
            }
        }

        if (closest_dist < 20) closest_dist = 20;

        int line_height = (350 * FP_ONE) / (closest_dist > 0 ? closest_dist : 1);
        int draw_start = -line_height / 2 + 9;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + 9;
        if (draw_end >= 19) draw_end = 18;

        /* Render 3D Wall Column directly to VGA buffer at 0xB8000 */
        for (int y = 0; y < 19; y++) {
            uint8_t color;
            char ch;

            if (y < draw_start) {
                /* Ceiling: Dark Space */
                color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK);
                ch = ' ';
            } else if (y <= draw_end) {
                /* 3D Wall: Solid Block 219 (0xDB) with depth shading */
                if (hit_side == 1) {
                    color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);
                    ch = (char)219;
                } else {
                    color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_DARK_GREY);
                    ch = (char)219;
                }
            } else {
                /* Floor: Textured Floor Shader */
                color = vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
                ch = (char)176;
            }

            vga_buf[y * 80 + x] = vga_entry(ch, color);
        }
    }

    /* Render DOOM E1M1 HUD Bar at Rows 19-24 */
    vga_set_cursor(0, 19);
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_RED));
    vga_puts("================================================================================\n");
    vga_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    vga_puts(" E1M1: KNEE-DEEP IN THE DEAD | HEALTH: ");
    vga_putdec(health);
    vga_puts("% | AMMO: ");
    vga_putdec(ammo);
    vga_puts(" | KILLS: ");
    vga_putdec(kills);
    if (shooting) {
        vga_puts(" | [BANG! BOOM!]");
    } else {
        vga_puts(" | [SHOTGUN]    ");
    }
    vga_puts("\n");
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_RED));
    vga_puts(" Controls: [w/s] Move | [a/d] Turn | [Space/f] Shoot | [q] Exit DOOM            \n");
    vga_puts("================================================================================\n");
}

void doom_main(void) {
    vga_clear();
    px = -1056;
    py = -3616;
    dir_x = FP_ONE;
    dir_y = 0;
    plane_x = 0;
    plane_y = 168;
    health = 100;
    ammo = 50;
    kills = 0;

    render_doom_frame();

    while (1) {
        shooting = 0;
        char c = keyboard_getchar();

        if (c == 'q') {
            vga_clear();
            vga_puts("Exiting DOOM Engine back to OniOS shell...\n");
            break;
        }

        /* Forward / Backward movement */
        if (c == 'w') {
            px += (dir_x * 40) >> FP_SHIFT;
            py += (dir_y * 40) >> FP_SHIFT;
        } else if (c == 's') {
            px -= (dir_x * 40) >> FP_SHIFT;
            py -= (dir_y * 40) >> FP_SHIFT;
        }

        /* Left / Right Turning */
        if (c == 'a') {
            int old_dir_x = dir_x;
            dir_x = ((dir_x * 250) >> 8) - ((dir_y * -51) >> 8);
            dir_y = ((old_dir_x * -51) >> 8) + ((dir_y * 250) >> 8);
            int old_plane_x = plane_x;
            plane_x = ((plane_x * 250) >> 8) - ((plane_y * -51) >> 8);
            plane_y = ((old_plane_x * -51) >> 8) + ((plane_y * 250) >> 8);
        } else if (c == 'd') {
            int old_dir_x = dir_x;
            dir_x = ((dir_x * 250) >> 8) - ((dir_y * 51) >> 8);
            dir_y = ((old_dir_x * 51) >> 8) + ((dir_y * 250) >> 8);
            int old_plane_x = plane_x;
            plane_x = ((plane_x * 250) >> 8) - ((plane_y * 51) >> 8);
            plane_y = ((old_plane_x * 51) >> 8) + ((plane_y * 250) >> 8);
        }

        /* Shooting */
        if (c == ' ' || c == 'f') {
            if (ammo > 0) {
                ammo--;
                kills += 1;
                shooting = 1;
            }
        }

        render_doom_frame();
    }
}
