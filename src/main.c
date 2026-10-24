#include <stdint.h>
#include <stdio.h>

#include "epd.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

static epd_image image1;
static epd_image image2;

static bool draw_to2 = true;
// Fixed poin with 24 fractional bits: the RP2040 has no FPU.
#define FIX(x) ((int32_t)((x) * (1 << 24)))

typedef bool (*update_cell)(uint8_t, bool);

bool update_conway(uint8_t neighbors, bool center) {
  int sum = __builtin_popcount(neighbors);
  if (sum <= 1) {
    return 0;
  } else if (sum == 2) {
    return center;
  } else if (sum == 3) {
    return 1;
  } else {
    return 0;
  }
}

static void update_all(update_cell func) {
  uint8_t *draw_to;
  uint8_t *last_frame;
  if (draw_to2) {
    draw_to = image2;
    last_frame = image1;
  } else {
    draw_to = image1;
    last_frame = image2;
  }
  for (int y = 0; y < EPD_WIDTH; y++) {
    for (int x = 0; x < EPD_HEIGHT; x++) {
      uint8_t neighbors = 0;
      neighbors |= epd_pixel_get(last_frame, x - 1, y - 1) << 0;
      neighbors |= epd_pixel_get(last_frame, x + 0, y - 1) << 1;
      neighbors |= epd_pixel_get(last_frame, x + 1, y - 1) << 2;
      neighbors |= epd_pixel_get(last_frame, x - 1, y + 0) << 3;
      neighbors |= epd_pixel_get(last_frame, x + 1, y + 0) << 4;
      neighbors |= epd_pixel_get(last_frame, x - 1, y + 1) << 5;
      neighbors |= epd_pixel_get(last_frame, x + 0, y + 1) << 6;
      neighbors |= epd_pixel_get(last_frame, x + 1, y + 1) << 7;
      bool new_pix = func(neighbors, epd_pixel_get(last_frame, x, y));
      epd_pixel_set(draw_to, x, y, new_pix);
    }
  }
  epd_show(draw_to);
}

static void draw_mandelbrot(void) {
  const int32_t scale = FIX(3.0 / EPD_WIDTH);
  const int32_t cx0 = FIX(-2.25), cy0 = -scale * (EPD_HEIGHT / 2);
  for (int y = 0; y < EPD_HEIGHT; y++) {
    int32_t cy = cy0 + scale * y;
    for (int x = 0; x < EPD_WIDTH; x++) {
      int32_t cx = cx0 + scale * x;
      int32_t zx = 0, zy = 0;
      int n = 0;
      for (; n < 64; n++) {
        int64_t xx = (int64_t)zx * zx >> 24, yy = (int64_t)zy * zy >> 24;
        if (xx + yy > FIX(4.0))
          break;
        zy = (int32_t)((2 * (int64_t)zx * zy) >> 24) + cy;
        zx = (int32_t)(xx - yy) + cx;
      }
      // The set itself in black, and alternating bands of escape time around
      // it.
      epd_pixel_set(image1, x, y, n == 64 || (n > 3 && n % 2 == 0));
    }
  }
}

int main(void) {
  stdio_init_all();
  // The Pico W's LED hangs off the radio chip.
  cyw43_arch_init();
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  // Give a serial monitor up to 3 s to connect, so the progress lines are seen.
  for (int i = 0; i < 30 && !stdio_usb_connected(); i++)
    sleep_ms(100);

  draw_mandelbrot();
  epd_show(image1);
  printf("done\n");

  while (true) {
    update_all(update_conway);
  }

  // Blink as a sign of life.
  for (bool on = false;; on = !on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
    sleep_ms(250);
  }
}
