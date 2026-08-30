#include "doom_engine.h"
#include "graphics.h"
#include "vga.h"
#include "keyboard.h"
#include "io.h"

#define MAP_WIDTH 16
#define MAP_HEIGHT 16

/* 3D Map Grid: 1 = Brick Wall, 2 = Door, 3 = Demon Spawn, 0 = Corridor */
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

/* Fixed-Point Math (Scaled by 256) */
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

static void draw_hud_text(const char* text, uint32_t x, uint32_t y, uint32_t color, uint32_t bg) {
    while (*text) {
        graphics_draw_char(*text++, x, y, color, bg);
        x += 8;
    }
}

static void draw_hud_num(int val, uint32_t x, uint32_t y, uint32_t color, uint32_t bg) {
    char buf[12];
    if (val == 0) {
        graphics_draw_char('0', x, y, color, bg);
        return;
    }
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (--i >= 0) {
        graphics_draw_char(buf[i], x, y, color, bg);
        x += 8;
    }
}

static void render_doom_frame_clean(void) {
    /* Render 3D Raycasted Columns across the entire screen width */
    for (uint32_t x = 0; x < graphics_width; x++) {
        int camera_x = (2 * x * FP_ONE / graphics_width) - FP_ONE;
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

        int line_height = (graphics_height * FP_ONE) / (perp_wall_dist > 0 ? perp_wall_dist : 1);
        int draw_start = -line_height / 2 + graphics_height / 2;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + graphics_height / 2;
        if (draw_end >= (int)graphics_height) draw_end = graphics_height - 1;

        /* Wall Shading Colors (32-bit RGB) */
        uint32_t wall_color;
        if (wall_type == 2) {
            wall_color = (side == 1) ? 0xAA5500 : 0xFFFF55; // Brown / Yellow
        } else if (wall_type == 3) {
            wall_color = (side == 1) ? 0xAA0000 : 0xFF5555; // Red / Light Red
        } else {
            wall_color = (side == 1) ? 0x00AAAA : 0x55FFFF; // Cyan / Light Cyan
        }

        /* Draw Vertical Column */
        for (int y = 0; y < draw_start; y++) {
            graphics_put_pixel(x, y, 0x000000); /* Ceiling: Black Space */
        }
        for (int y = draw_start; y <= draw_end; y++) {
            graphics_put_pixel(x, y, wall_color); /* 3D Wall */
        }
        for (int y = draw_end + 1; y < (int)graphics_height; y++) {
            graphics_put_pixel(x, y, 0x333333); /* Floor: Dark Grey */
        }
    }

    /* Render DOOM Status Bar HUD at bottom of screen */
    uint32_t hud_y = graphics_height - 50;
    graphics_draw_rect(0, hud_y, graphics_width, 50, 0xAA0000); // Red background
    graphics_draw_rect(5, hud_y + 5, graphics_width - 10, 40, 0x000000); // Inner black border

    uint32_t text_y = hud_y + 16;
    draw_hud_text(" E1M1: KNEE-DEEP IN THE DEAD ", 20, text_y, 0xFFFFFF, 0x000000);
    draw_hud_text("| HEALTH: ", 260, text_y, 0xFFFFFF, 0x000000);
    draw_hud_num(health, 340, text_y, 0xFFFFFF, 0x000000);
    draw_hud_text("% | AMMO: ", 370, text_y, 0xFFFFFF, 0x000000);
    draw_hud_num(ammo, 450, text_y, 0xFFFFFF, 0x000000);
    draw_hud_text(" | KILLS: ", 470, text_y, 0xFFFFFF, 0x000000);
    draw_hud_num(kills, 550, text_y, 0xFFFFFF, 0x000000);
    
    if (shooting) {
        draw_hud_text(" | [BOOM! FIRE]", 580, text_y, 0xFFFF55, 0x000000);
    } else {
        draw_hud_text(" | [SHOTGUN]  ", 580, text_y, 0xFFFFFF, 0x000000);
    }

    /* Render 3D Shotgun Weapon Sprite blockily in the center bottom */
    uint32_t gun_color = shooting ? 0xFFFF55 : 0xFFFFFF; // Yellow flash or white
    const char* gun_str = shooting ? "BANG!" : "[SHOTGUN]";
    draw_hud_text(gun_str, graphics_width / 2 - 40, hud_y - 20, gun_color, 0x000000);
    graphics_draw_rect(graphics_width / 2 - 10, hud_y - 40, 20, 40, 0x777777); // Gun barrel
}

void doom_main(void) {
    graphics_clear(0x000000);
    px = 2 * FP_ONE + 128;
    py = 2 * FP_ONE + 128;
    dir_x = FP_ONE;
    dir_y = 0;
    plane_x = 0;
    plane_y = 168;
    health = 100;
    ammo = 50;
    kills = 0;

    render_doom_frame_clean();

    while (1) {
        shooting = 0;
        char c = keyboard_getchar();

        if (c == 'q') {
            graphics_clear(0x000000);
            draw_hud_text("Exiting DOOM Engine back to OniOS shell...", 10, 10, 0x55FF55, 0x000000);
            /* Reset terminal emulator state */
            vga_init(0); /* Black bg, black fg (will be overridden by shell anyway) */
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

        render_doom_frame_clean();
    }
}
