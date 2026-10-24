#include <stdio.h>

#include "epd.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

static epd_image image;

// Fixed point with 24 fractional bits: the RP2040 has no FPU.
#define FIX(x) ((int32_t)((x) * (1 << 24)))

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
      epd_pixel(image, x, y, n == 64 || (n > 3 && n % 2 == 0));
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
  epd_show(image);
  printf("done\n");

  // Blink as a sign of life.
  for (bool on = false;; on = !on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
    sleep_ms(250);
  }
}
