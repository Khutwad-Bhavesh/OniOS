#include "doom_engine.h"
#include "vga.h"
#include "keyboard.h"
#include "io.h"

#define MAP_WIDTH 16
#define MAP_HEIGHT 16

/* 3D Map Grid: 1 = Wall, 0 = Empty Corridor, 2 = Door, 3 = Demon Spawn */
static const int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,0,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
    {1,1,1,0,1,0,3,0,0,0,1,0,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,0,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,1,0,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,0,1,1,2,1,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,3,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

/* Integer Fixed-Point Arithmetic (Scaled by 256) */
#define FP_SHIFT 8
#define FP_ONE   (1 << FP_SHIFT)

static int px = 2 * FP_ONE + 128; // 2.5
static int py = 2 * FP_ONE + 128; // 2.5
static int dir_x = FP_ONE;        // 1.0
static int dir_y = 0;             // 0.0
static int plane_x = 0;           // 0.0
static int plane_y = 168;         // ~0.66 * 256

static int health = 100;
static int ammo = 50;
static int kills = 0;
static int shooting = 0;

static void render_doom_frame(void) {
    uint16_t* const vga_buf = (uint16_t*) 0xB8000;

    /* Render 3D Raycasted Walls */
    for (int x = 0; x < 80; x++) {
        int camera_x = (2 * x * FP_ONE / 80) - FP_ONE;
        int ray_dir_x = dir_x + ((plane_x * camera_x) >> FP_SHIFT);
        int ray_dir_y = dir_y + ((plane_y * camera_x) >> FP_SHIFT);

        int map_x = px >> FP_SHIFT;
        int map_y = py >> FP_SHIFT;

        int delta_dist_x = (ray_dir_x == 0) ? 32767 : ((FP_ONE * FP_ONE) / (ray_dir_x < 0 ? -ray_dir_x : ray_dir_x));
        int delta_dist_y = (ray_dir_y == 0) ? 32767 : ((FP_ONE * FP_ONE) / (ray_dir_y < 0 ? -ray_dir_y : ray_dir_y));

        int side_dist_x, side_dist_y;
        int step_x, step_y;
        int hit = 0;
        int side = 0;

        if (ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = ((px - (map_x << FP_SHIFT)) * delta_dist_x) >> FP_SHIFT;
        } else {
            step_x = 1;
            side_dist_x = ((((map_x + 1) << FP_SHIFT) - px) * delta_dist_x) >> FP_SHIFT;
        }

        if (ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = ((py - (map_y << FP_SHIFT)) * delta_dist_y) >> FP_SHIFT;
        } else {
            step_y = 1;
            side_dist_y = ((((map_y + 1) << FP_SHIFT) - py) * delta_dist_y) >> FP_SHIFT;
        }

        /* DDA Raycast loop */
        while (hit == 0) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }
            if (map_x >= 0 && map_x < MAP_WIDTH && map_y >= 0 && map_y < MAP_HEIGHT) {
                if (map[map_y][map_x] > 0) hit = 1;
            } else {
                hit = 1;
            }
        }

        int perp_wall_dist;
        if (side == 0) perp_wall_dist = side_dist_x - delta_dist_x;
        else          perp_wall_dist = side_dist_y - delta_dist_y;

        if (perp_wall_dist < 32) perp_wall_dist = 32;

        int line_height = (18 * FP_ONE) / (perp_wall_dist > 0 ? perp_wall_dist : 1);
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
                /* 3D Wall: Solid Block 219 (0xDB) with shading */
                if (side == 1) {
                    color = vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_DARK_GREY);
                    ch = (char)219;
                } else {
                    color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);
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

    /* Render DOOM HUD Bar at Rows 19-24 */
    vga_set_cursor(0, 19);
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_RED));
    vga_puts("================================================================================\n");
    vga_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    vga_puts(" HEALTH: ");
    vga_putdec(health);
    vga_puts("% | AMMO: ");
    vga_putdec(ammo);
    vga_puts(" | KILLS: ");
    vga_putdec(kills);
    if (shooting) {
        vga_puts(" | WEAPON: [BANG! BOOM!]    ");
    } else {
        vga_puts(" | WEAPON: [SHOTGUN]        ");
    }
    vga_puts("\n");
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_RED));
    vga_puts(" Controls: [w/s] Move | [a/d] Turn | [Space/f] Shoot | [q] Exit DOOM            \n");
    vga_puts("================================================================================\n");
}

void doom_main(void) {
    vga_clear();
    px = 2 * FP_ONE + 128;
    py = 2 * FP_ONE + 128;
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
            int new_x = px + ((dir_x * 80) >> FP_SHIFT);
            int new_y = py + ((dir_y * 80) >> FP_SHIFT);
            if (map[new_y >> FP_SHIFT][new_x >> FP_SHIFT] == 0) {
                px = new_x;
                py = new_y;
            }
        } else if (c == 's') {
            int new_x = px - ((dir_x * 80) >> FP_SHIFT);
            int new_y = py - ((dir_y * 80) >> FP_SHIFT);
            if (map[new_y >> FP_SHIFT][new_x >> FP_SHIFT] == 0) {
                px = new_x;
                py = new_y;
            }
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
