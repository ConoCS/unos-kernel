#include "../boot/bootinfo.h"
#include "../Include/graphic.h"
#include "../Include/com.h"
#include "../file/extension/psf.h"
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

    // Validate screen_pitch to ensure it matches screen_width * sizeof(uint32_t)
    if (screen_pitch != screen_width * sizeof(uint32_t)) {
        serial_print("[Warning] screen_pitch tidak sesuai dengan screen_width\n");
        screen_pitch = screen_width * sizeof(uint32_t); // Correct the pitch value
    }

    if(framebuffer != NULL && screen_width != 0 && screen_height != 0 && screen_pitch != 0) {
        serial_print("Succesfully connected to framebuffer\n");
    } else {
        serial_print("Failed connected to framebuffer\n");
    }
    serial_print("Width: "); serial_print_hex((uint64_t)screen_width); serial_print("\n");
    serial_print("Height: "); serial_print_hex((uint64_t)screen_height); serial_print("\n");
    serial_print("Pitch: "); serial_print_hex((uint64_t)screen_pitch); serial_print("\n");
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

void draw_glyph(int x, int y, const uint8_t *glyph, int width, int height, uint32_t color) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            if (glyph[j] & (1 << (7 - i))) { // Periksa bit untuk pixel aktif
                put_pixel(x + i, y + j, color);
            }
        }
    }
}

void draw_text_from_psf(const char *path, int x, int y, const char *text, uint32_t color) {
    serial_printf("[DEBUG] Loading PSF file: %s\n", path);
    void *psf_file = load_psf_file(path);
    if (!psf_file) {
        serial_printf("[Error] Failed to load PSF file: %s\n", path);
        return;
    }

    serial_printf("[DEBUG] PSF file loaded successfully, Address: %p\n", psf_file);
    const uint8_t *glyph_data = parse_psf(psf_file);
    if (!glyph_data) {
        serial_printf("[Error] Failed to parse PSF file: %s\n", path);
        return;
    }

    serial_printf("[DEBUG] Glyph data parsed successfully, Address: %p\n", glyph_data);
    PSFHeader *header = (PSFHeader*)psf_file;
    serial_printf("[DEBUG] PSF Header: Glyph Width=%d, Glyph Height=%d, Glyph Size=%d\n",
                  header->glyph_width, header->glyph_height, header->glyph_size);

    int glyph_width = header->glyph_width;
    int glyph_height = header->glyph_height;

    while (*text) {
        const uint8_t *glyph = glyph_data + (*text * header->glyph_size);
        serial_printf("[DEBUG] Drawing character '%c' at position (%d, %d)\n", *text, x, y);
        serial_printf("[DEBUG] Glyph Address: %p, Width=%d, Height=%d\n", glyph, glyph_width, glyph_height);
        draw_glyph(x, y, glyph, glyph_width, glyph_height, color);
        x += glyph_width; // Geser posisi untuk karakter berikutnya
        text++;
    }
}