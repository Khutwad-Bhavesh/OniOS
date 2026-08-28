#include "doom_engine.h"
#include "vga.h"
#include "vga13.h"
#include "keyboard.h"
#include "io.h"

#define MAP_WIDTH 16
#define MAP_HEIGHT 16

/* 3D Map Grid: 1 = Brick Wall, 2 = Door, 3 = Demon Spawn, 0 = Empty Corridor */
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

static void render_doom_frame_3d(void) {
    uint8_t* const vmem = VGA13_MEMORY;

    /* Render 320 3D Pixel Raycasted Columns */
    for (int x = 0; x < 320; x++) {
        int camera_x = (2 * x * FP_ONE / 320) - FP_ONE;
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
        int wall_type = 1;

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
                if (map[map_y][map_x] > 0) {
                    hit = 1;
                    wall_type = map[map_y][map_x];
                }
            } else {
                hit = 1;
            }
        }

        int perp_wall_dist;
        if (side == 0) perp_wall_dist = side_dist_x - delta_dist_x;
        else          perp_wall_dist = side_dist_y - delta_dist_y;

        if (perp_wall_dist < 32) perp_wall_dist = 32;

        int line_height = (160 * FP_ONE) / (perp_wall_dist > 0 ? perp_wall_dist : 1);
        int draw_start = -line_height / 2 + 80;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + 80;
        if (draw_end >= 160) draw_end = 159;

        /* Wall 256-Color Palette Selection */
        uint8_t wall_color;
        if (wall_type == 2) {
            wall_color = (side == 1) ? 24 : 28; // Steel Blue Door
        } else if (wall_type == 3) {
            wall_color = (side == 1) ? 40 : 44; // Demon Red Spawn
        } else {
            wall_color = (side == 1) ? 160 : 164; // DOOM Brown/Grey Brick
        }

        /* Render column pixels directly in Mode 13h memory 0xA0000 */
        for (int y = 0; y < 160; y++) {
            uint8_t pixel_color;
            if (y < draw_start) {
                /* Ceiling: Dark Blue Sky */
                pixel_color = 17;
            } else if (y <= draw_end) {
                /* 3D Wall with vertical texture stripe shading */
                pixel_color = wall_color + (y % 4);
            } else {
                /* Floor: Ground Brown */
                pixel_color = 136 + (y % 4);
            }
            vmem[y * 320 + x] = pixel_color;
        }
    }

    /* Render 3D Shotgun Weapon Sprite at center bottom */
    int gun_x = 135;
    int gun_y = 120;
    vga13_draw_rect(gun_x, gun_y, 50, 40, 20);      // Gun Barrel (Dark Metal)
    vga13_draw_rect(gun_x + 15, gun_y - 15, 20, 15, 8); // Sight
    if (shooting) {
        /* Muzzle Flash SFX Explosion */
        vga13_draw_rect(gun_x + 10, gun_y - 30, 30, 20, 44); // Bright Red/Yellow Fire
        vga13_draw_rect(gun_x + 15, gun_y - 25, 20, 10, 14);
    }

    /* Render Original 1993 DOOM Red HUD Bar (Rows 160-200) */
    vga13_draw_rect(0, 160, 320, 40, 4);  // Red HUD Background
    vga13_draw_rect(0, 160, 320, 2, 14);  // Yellow Border Top
    vga13_draw_rect(10, 168, 60, 24, 0);  // Health Box
    vga13_draw_rect(80, 168, 60, 24, 0);  // Ammo Box
    vga13_draw_rect(150, 168, 60, 24, 0); // Kills Box
    vga13_draw_rect(220, 168, 90, 24, 0); // Level Title Box
}

void doom_main(void) {
    /* Switch VGA hardware to 320x200 256-color graphics Mode 13h */
    vga13_init();
    vga13_clear(0);

    px = 2 * FP_ONE + 128;
    py = 2 * FP_ONE + 128;
    dir_x = FP_ONE;
    dir_y = 0;
    plane_x = 0;
    plane_y = 168;
    health = 100;
    ammo = 50;
    kills = 0;

    render_doom_frame_3d();

    while (1) {
        shooting = 0;
        char c = keyboard_getchar();

        if (c == 'q') {
            /* Switch back to 80x25 VGA text mode */
            vga_init(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE));
            vga_clear();
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

        render_doom_frame_3d();
    }
}
