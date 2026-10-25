host := "inkpico.local"

build:
    cmake -S . -B build -G Ninja
    ninja -C build

# Puts an app on the Pico over Wi-Fi: image or life.
flash app: build
    "$PICO_OTA_PATH/tools/ota" flash {{ host }} build/{{ app }}.bin

# Shows an HTML page, a Typst document or a picture on the image app; see tools/show.
show file *flags:
    tools/show {{ flags }} {{ quote(file) }} {{ host }}

# The program's printf output; tio reconnects across reflashes.
monitor:
    tio $(ls /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null | head -1)

# Everything CI runs.
ci:
    nix fmt -- --ci
    just build
