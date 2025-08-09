#include <unoskrnl.h>


extern GOP_BOOT_INFO *gopInfo;

// Global framebuffer data
uint32_t *framebuffer = 0;
uint32_t screen_width = 0;
uint32_t screen_height = 0;
uint32_t screen_pitch = 0;

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

void drawfullscreen(uint32_t color){
    drawrect(0, 0, screen_width, screen_height, color);
}

void draw_glyph(int x, int y, const uint8_t *glyph, int width, int height, uint32_t color) {
    int bytes_per_row = (width + 7) / 8;

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int byte_index = j * bytes_per_row + (i / 8);
            int bit_index = 7 - (i % 8);

            if (glyph[byte_index] & (1 << bit_index)) {
                put_pixel(x + i, y + j, color);
            }
        }
    }
}

void RaiseKernelPanicError(uint64_t error_code, KernelPanic error_type) {
    (void)error_code;
    drawfullscreen(0xFFFF0000);
    ResetCursorX();
    ResetCursorY();
    DrawSimplePSFText("EXCEPTION: EXCEPTION HAPPENED\n",  0xFFFFFFFF);
    DrawSimplePSFText("PLEASE HARD RESET YOUR COMPUTER. THE KERNEL IS HALTING\n", 0xFFFFFFFF);
    switch(error_type) {
        case MEMORY_PAGE_FAULT_PAGE_UNREADY:
            DrawSimplePSFText("ERROR: MEMORY_PAGE_FAULT_PAGE_UNREADY\n", 0xFFFFFFFF);
            break;
        case WATCHDOG_TIMEOUT_TIMER_EXCEPTION:
            DrawSimplePSFText("ERROR: WATCHDOG_TIMEOUT_TIMER_EXCEPTION\n", 0xFFFFFFFF);
            break;
        case CPU_FAULT_ACTIVATE_GENERAL_PROTECTION:
            DrawSimplePSFText("ERROR: CPU_FAULT_ACTIVATE_GENERAL_PROTECTION\n", 0xFFFFFFFF);
            break;
        case FAILED_INITIALIZATION_USERLAND:
            serial_printf("ERROR: FAILED_INITIALIZATION_USERLAND\n", 0xFFFFFFFF);
            break;
        default:
            DrawSimplePSFText("ERROR: UNKNOWN_EXCEPTION\n", 0xFFFFFFFF);
            break;
    }
    if(error_code) {
        serial_printf("ERROR CODE: %X\n", error_code);
    }

    short_pause();

     __asm__ __volatile__ (
        "cli\n\t"
        "mov $0xFE, %%al\n\t"
        "out %%al, $0x64\n\t"
        "hlt\n\t"
        "jmp .\n\t"
        :
        :
        : "al"
    );

    __builtin_unreachable();

    AcpiShutdown();
}
