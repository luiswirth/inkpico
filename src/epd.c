// Command sequence from waveshareteam/Pico_ePaper_Code,
// c/lib/e-Paper/EPD_5in83_V2.c.

#include "epd.h"

#include <stdio.h>
#include <stdlib.h>

#include "hardware/spi.h"
#include "pico/stdlib.h"

// Pins of the Waveshare Pico-ePaper boards.
#define PIN_DC 8
#define PIN_CS 9
#define PIN_CLK 10
#define PIN_MOSI 11
#define PIN_RST 12
#define PIN_BUSY 13
#define EPD_SPI spi1

void epd_pixel_set(epd_image img, int x, int y, bool black) {
  x = (x + EPD_WIDTH) % EPD_WIDTH;
  y = (y + EPD_HEIGHT) % EPD_HEIGHT;
  uint8_t *byte = &img[y * EPD_ROW_BYTES + x / 8];
  uint8_t bit = 0x80 >> (x % 8);
  *byte = black ? *byte | bit : *byte & ~bit;
}

bool epd_pixel_get(epd_image img, int x, int y) {
  x = (x + EPD_WIDTH) % EPD_WIDTH;
  y = (y + EPD_HEIGHT) % EPD_HEIGHT;
  uint8_t bit = 0x80 >> (x % 8);
  return (img[y * EPD_ROW_BYTES + x / 8] & bit) != 0;
}

static void send(bool data, uint8_t byte) {
  gpio_put(PIN_DC, data);
  gpio_put(PIN_CS, 0);
  spi_write_blocking(EPD_SPI, &byte, 1);
  gpio_put(PIN_CS, 1);
}

static void command(uint8_t c) { send(false, c); }
static void data(uint8_t d) { send(true, d); }

// BUSY is low while the controller works; polling needs the get-status command.
// A refresh takes a few seconds; one that ends at once never reached the
// controller.
static void wait_idle(void) {
  printf("epd: busy...");
  int n = 0;
  do {
    command(0x71);
    sleep_ms(10);
    if (++n % 100 == 0)
      printf(" %ds", n / 100);
  } while (!gpio_get(PIN_BUSY));
  printf(" idle\n");
}

static void setup_pins(void) {
  static bool done = false;
  if (done)
    return;
  done = true;

  const uint outputs[] = {PIN_RST, PIN_DC, PIN_CS};
  for (int i = 0; i < 3; i++) {
    gpio_init(outputs[i]);
    gpio_set_dir(outputs[i], GPIO_OUT);
  }
  gpio_put(PIN_CS, 1);
  gpio_init(PIN_BUSY);
  gpio_set_dir(PIN_BUSY, GPIO_IN);
  spi_init(EPD_SPI, 4000 * 1000);
  gpio_set_function(PIN_CLK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
}

// The reset also wakes the controller from deep sleep.
static void init(void) {
  gpio_put(PIN_RST, 1);
  sleep_ms(200);
  gpio_put(PIN_RST, 0);
  sleep_ms(5);
  gpio_put(PIN_RST, 1);
  sleep_ms(200);

  command(0x01); // power setting
  data(0x07);
  data(0x07);
  data(0x3f);
  data(0x3f);
  command(0x04); // power on
  sleep_ms(100);
  wait_idle();
  command(0x00); // panel setting: black/white, LUT from OTP
  data(0x1f);
  command(0x61); // resolution
  data(EPD_WIDTH >> 8);
  data(EPD_WIDTH & 0xff);
  data(EPD_HEIGHT >> 8);
  data(EPD_HEIGHT & 0xff);
  command(0x15);
  data(0x00);
  command(0x50); // VCOM and data interval
  data(0x10);
  data(0x07);
  command(0x60); // TCON
  data(0x22);
}

static void deep_sleep(void) {
  command(0x02); // power off
  wait_idle();
  command(0x07); // deep sleep
  data(0xa5);
}

void epd_show(const epd_image img) {
  setup_pins();
  init();
  command(0x10); // old data
  for (int i = 0; i < EPD_ROW_BYTES * EPD_HEIGHT; i++)
    data(0x00);
  command(0x13); // new data
  for (int i = 0; i < EPD_ROW_BYTES * EPD_HEIGHT; i++)
    data(img[i]);
  command(0x12); // refresh
  sleep_ms(100);
  wait_idle();
  deep_sleep();
}
