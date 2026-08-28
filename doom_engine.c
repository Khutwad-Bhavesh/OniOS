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

/* Fixed-point math approximation for bare-metal raycasting */
static float px = 2.5f;
static float py = 2.5f;
static float dir_x = 1.0f;
static float dir_y = 0.0f;
static float plane_x = 0.0f;
static float plane_y = 0.66f;

static int health = 100;
static int ammo = 50;
static int kills = 0;
static int shooting = 0;

static void render_doom_frame(void) {
    vga_clear();

    /* Render 3D Raycasted Walls */
    for (int x = 0; x < 80; x++) {
        float camera_x = 2.0f * x / 80.0f - 1.0f;
        float ray_dir_x = dir_x + plane_x * camera_x;
        float ray_dir_y = dir_y + plane_y * camera_x;

        int map_x = (int)px;
        int map_y = (int)py;

        float delta_dist_x = (ray_dir_x == 0) ? 1e30f : (ray_dir_x > 0 ? 1.0f / ray_dir_x : -1.0f / ray_dir_x);
        float delta_dist_y = (ray_dir_y == 0) ? 1e30f : (ray_dir_y > 0 ? 1.0f / ray_dir_y : -1.0f / ray_dir_y);

        float side_dist_x, side_dist_y;
        int step_x, step_y;
        int hit = 0;
        int side = 0;

        if (ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = (px - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0f - px) * delta_dist_x;
        }

        if (ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = (py - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0f - py) * delta_dist_y;
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

        float perp_wall_dist;
        if (side == 0) perp_wall_dist = (side_dist_x - delta_dist_x);
        else          perp_wall_dist = (side_dist_y - delta_dist_y);

        if (perp_wall_dist < 0.1f) perp_wall_dist = 0.1f;

        int line_height = (int)(18.0f / perp_wall_dist);
        int draw_start = -line_height / 2 + 9;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + 9;
        if (draw_end >= 18) draw_end = 17;

        /* Wall Color Shading */
        uint8_t wall_color;
        if (side == 1) {
            wall_color = vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLUE);
        } else {
            wall_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);
        }

        /* Render column pixels */
        for (int y = 0; y < 18; y++) {
            if (y < draw_start) {
                /* Ceiling */
                vga_set_color(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BLUE));
            } else if (y <= draw_end) {
                /* Wall */
                vga_set_color(wall_color);
            } else {
                /* Floor */
                vga_set_color(vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLUE));
            }
            /* Render line */
        }
    }

    /* Render DOOM HUD Bar */
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
        vga_puts(" | WEAPON: 🔥 [BANG! SFX] 🔥");
    } else {
        vga_puts(" | WEAPON: [🔫 SHOTGUN]");
    }
    vga_puts("\n");
    vga_set_color(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_RED));
    vga_puts(" Controls: [w/s] Move | [a/d] Turn | [Space/f] Shoot | [q] Exit DOOM\n");
    vga_puts("================================================================================\n");
}

void doom_main(void) {
    vga_clear();
    px = 2.5f;
    py = 2.5f;
    dir_x = 1.0f;
    dir_y = 0.0f;
    plane_x = 0.0f;
    plane_y = 0.66f;
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
            float new_x = px + dir_x * 0.4f;
            float new_y = py + dir_y * 0.4f;
            if (map[(int)new_y][(int)new_x] == 0) {
                px = new_x;
                py = new_y;
            }
        } else if (c == 's') {
            float new_x = px - dir_x * 0.4f;
            float new_y = py - dir_y * 0.4f;
            if (map[(int)new_y][(int)new_x] == 0) {
                px = new_x;
                py = new_y;
            }
        }

        /* Left / Right Turning */
        if (c == 'a') {
            float rot = -0.2f;
            float old_dir_x = dir_x;
            dir_x = dir_x * 0.98f - dir_y * (-0.2f);
            dir_y = old_dir_x * (-0.2f) + dir_y * 0.98f;
            float old_plane_x = plane_x;
            plane_x = plane_x * 0.98f - plane_y * (-0.2f);
            plane_y = old_plane_x * (-0.2f) + plane_y * 0.98f;
        } else if (c == 'd') {
            float rot = 0.2f;
            float old_dir_x = dir_x;
            dir_x = dir_x * 0.98f - dir_y * 0.2f;
            dir_y = old_dir_x * 0.2f + dir_y * 0.98f;
            float old_plane_x = plane_x;
            plane_x = plane_x * 0.98f - plane_y * 0.2f;
            plane_y = old_plane_x * 0.2f + plane_y * 0.98f;
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
