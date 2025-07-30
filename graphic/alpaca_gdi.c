#include "../boot/bootinfo.h"
#include "../Include/graphic.h"
#include "../Include/com.h"
#include <stdint.h>

#define NULL 0

extern GOP_BOOT_INFO *gopInfo;

// Global framebuffer data
static uint32_t *framebuffer = 0;
static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static uint32_t screen_pitch = 0;

void connect_to_framebuffer() {
    framebuffer = (uint32_t*)gopInfo->gop_framebase;
    screen_width = gopInfo->gop_width;
    screen_height = gopInfo->gop_height;
    screen_pitch = gopInfo->gop_pitch;

    if(framebuffer != NULL && screen_width != NULL && screen_height != NULL && screen_pitch != NULL) {
        serial_print("Succesfully connected to framebuffer\n");
    } else {
        serial_print("Failed connected to framebuffer\n");
    }
    serial_print("Width: "); serial_print_hex((uint64_t)screen_width); serial_print("\n");
    serial_print("Height: "); serial_print_hex((uint64_t)screen_height); serial_print("\n");
    serial_print("Pitch "); serial_print_hex((uint64_t)screen_pitch); serial_print("\n");
}

void put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= (int)screen_width || y >= (int)screen_height) return;

    uint32_t pixel_index = (y * (screen_pitch / 4)) + x;
    framebuffer[pixel_index] = color;
}

int drawrect(int x, int y, int w, int h, uint32_t color) {
    if(x < 0 || y < 0 || w <= 0 || h <= 0) return -1;
    if(x + w > (int)screen_width || y + h > (int)screen_height) return -2;
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            put_pixel(x + i, y + j, color);
        }
    }

    return 0;
}

void drawfullscreen(){
    drawrect(0, 0, screen_width, screen_height, 0x000000FF);
}