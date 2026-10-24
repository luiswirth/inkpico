# inkpico

C firmware for a Raspberry Pi Pico W (RP2040) driving a Waveshare 5.83" e-paper V2
(648x480, black and white), on the Pico SDK.

- The Pico is on the Mac's USB, so the work happens there.
- `src/epd.c` is the display driver, with the pins; `src/main.c` draws.
- The `justfile` builds, flashes and opens the serial monitor.
  Flashing needs no BOOTSEL as long as the running program has USB serial.
