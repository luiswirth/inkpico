# inkpico

C firmware for a Raspberry Pi Pico W (RP2040) driving a Waveshare 5.83" e-paper V2
(648x480, black and white), as apps on `pico-ota`.

- `src/epd.c` is the display driver, with the pins; every other file in `src/` is an app.
- `src/image.c` is the foundation: it shows a picture sent over HTTP, nothing more.
  What makes pictures is built on top of it.
- `pico-ota` is a flake input, as `PICO_OTA_PATH` in the dev shell; pointing it at `../pico-ota` builds against that checkout.
- Pictures are made with Typst: `tools/show` renders a Typst document or a picture file at the panel's size and sends it to `image`.
- The `justfile` builds, flashes an app over Wi-Fi, and wraps `tools/show`.
