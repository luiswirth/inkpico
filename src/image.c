// Shows a picture on the e-paper, over HTTP:
//   PUT /image  shows the picture in the body, and answers once it is shown
//   GET /image  the picture shown
// A picture is EPD_WIDTH x EPD_HEIGHT pixels, one bit each, row by row from
// the top, the most significant bit leftmost, 1 black: the raster of a PBM
// (P4) file. It is kept in flash and shown again after a restart.
#include <stdio.h>
#include <string.h>

#include "epd.h"
#include "hardware/flash.h"
#include "net.h"
#include "pico/stdlib.h"

// The app's flash holds a mark, then the picture in the sectors after it.
// The mark says the picture is complete.
#define MARK 0x65726970u
#define KEPT (NET_DATA + FLASH_SECTOR_SIZE)

static bool kept(void) { return *(const uint32_t *)NET_DATA == MARK; }

// The mark goes last, so that a picture cut off by power loss is not taken.
static bool keep(const uint8_t *img) {
  const uint32_t none = 0, mark = MARK;
  return net_data_write(0, &none, sizeof none) &&
         net_data_write(FLASH_SECTOR_SIZE, img, sizeof(epd_image)) &&
         net_data_write(0, &mark, sizeof mark);
}

static void handle(const net_request *req) {
  bool get = !strcmp(req->method, "GET"), put = !strcmp(req->method, "PUT");
  if (!strcmp(req->path, "/") && get) {
    net_reply(200, "image: GET or PUT /image\n", 25);
  } else if (strcmp(req->path, "/image")) {
    net_reply(404, "not found\n", 10);
  } else if (get) {
    if (kept())
      net_reply(200, KEPT, sizeof(epd_image));
    else
      net_reply(404, "no picture yet\n", 15);
  } else if (!put) {
    net_reply(405, "GET or PUT\n", 11);
  } else if (req->length != sizeof(epd_image)) {
    char msg[96];
    int n = snprintf(msg, sizeof msg,
                     "a picture is %u bytes: %dx%d pixels, 1 bit each\n",
                     (unsigned)sizeof(epd_image), EPD_WIDTH, EPD_HEIGHT);
    net_reply(422, msg, n);
  } else if (!keep(req->body)) {
    net_reply(500, "flash write failed\n", 19);
  } else {
    epd_show(KEPT);
    net_reply(200, "shown\n", 6);
  }
}

int main(void) {
  stdio_init_all();
  net_init();
  // The panel keeps its picture without power, but another app may have
  // drawn on it since.
  if (kept())
    epd_show(KEPT);
  for (;;) {
    net_request req;
    if (net_wait(&req, 1000))
      handle(&req);
  }
}
