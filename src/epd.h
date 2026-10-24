// Waveshare 5.83" e-paper V2: 648x480, black and white.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define EPD_WIDTH 648
#define EPD_HEIGHT 480

// One bit per pixel, row by row, most significant bit leftmost; 1 is black.
#define EPD_ROW_BYTES (EPD_WIDTH / 8)
typedef uint8_t epd_image[EPD_ROW_BYTES * EPD_HEIGHT];

void epd_pixel_set(epd_image img, int x, int y, bool black);
bool epd_pixel_get(epd_image img, int x, int y);

// Wakes the display, shows the image (a full refresh, a few seconds), and puts
// it back to sleep. The image stays on the screen without power.
void epd_show(const epd_image img);
