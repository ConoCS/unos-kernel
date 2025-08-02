#pragma once
#include <stdint.h>

void put_pixel(int x, int y, uint32_t color);
void connect_to_framebuffer();
int drawrect(int x, int y, int w, int h, uint32_t color);
void drawfullscreen(uint32_t color);
void RaiseKernelPanicError(uint64_t error_code, int error_type);
void draw_glyph(int x, int y, const uint8_t *glyph, int width, int height, uint32_t color);
void InitPSFFontGraphic();
void PreparingGlobalPSFFont();
void DrawSimplePSFText(const char *text, int x, int y, uint32_t color);