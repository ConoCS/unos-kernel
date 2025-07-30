#pragma once
#include <stdint.h>

void put_pixel(int x, int y, uint32_t color);
void connect_to_framebuffer();
int drawrect(int x, int y, int w, int h, uint32_t color);
void drawfullscreen();