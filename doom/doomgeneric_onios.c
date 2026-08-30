#include "doomgeneric.h"
#include "doomkeys.h"
#include "../vga.h"
#include "../keyboard.h"
#include "../timer.h"
#include "../graphics.h"

void DG_Init(void) {
    // Nothing to do for init, VESA is already set up by multiboot
}

void DG_DrawFrame(void) {
    // DG_ScreenBuffer contains DOOMGENERIC_RESX * DOOMGENERIC_RESY pixels (usually 640x400)
    // We need to blit this to our 800x600 VESA framebuffer
    // To keep it simple, we will draw it centered.
    
    int start_x = (800 - DOOMGENERIC_RESX) / 2;
    int start_y = (600 - DOOMGENERIC_RESY) / 2;
    
    uint32_t* src = (uint32_t*)DG_ScreenBuffer;
    
    for (int y = 0; y < DOOMGENERIC_RESY; y++) {
        for (int x = 0; x < DOOMGENERIC_RESX; x++) {
            uint32_t color = src[y * DOOMGENERIC_RESX + x];
            graphics_put_pixel(start_x + x, start_y + y, color);
        }
    }
}

void DG_SleepMs(uint32_t ms) {
    timer_sleep(ms);
}

uint32_t DG_GetTicksMs(void) {
    return timer_get_uptime_ms();
}

int DG_GetKey(int* pressed, unsigned char* doomKey) {
    char c;
    if (keyboard_get_event(pressed, &c)) {
        if (c == 'w') *doomKey = KEY_UPARROW;
        else if (c == 's') *doomKey = KEY_DOWNARROW;
        else if (c == 'a') *doomKey = KEY_STRAFE_L;
        else if (c == 'd') *doomKey = KEY_STRAFE_R;
        else if (c == ' ') *doomKey = KEY_FIRE;
        else if (c == 'e') *doomKey = KEY_USE;
        else if (c == '\n') *doomKey = KEY_ENTER;
        else if (c == '\t') *doomKey = KEY_TAB;
        else if (c == 27) *doomKey = KEY_ESCAPE; // Esc
        else *doomKey = c;
        return 1;
    }
    return 0;
}

void DG_SetWindowTitle(const char * title) {
    // We don't have a windowing system, so just print to top left
    vga_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    // We can't print easily without messing up graphics mode if we are in it,
    // but graphics_draw_char could be used. Do nothing for now.
}
