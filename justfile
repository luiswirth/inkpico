host := "inkpico.local"

build:
    cmake -S . -B build -G Ninja
    ninja -C build

# Puts an app on the Pico over Wi-Fi: image or life.
flash app: build
    "$PICO_OTA_PATH/tools/ota" flash {{ host }} build/{{ app }}.bin

# Shows a picture file of any format on the image app: fitted to the panel,
# dithered to black and white, and stripped of its PBM header.
show file:
    magick {{ quote(file) }} -resize 648x480 -background white -gravity center \
        -extent 648x480 -monochrome pbm:- | tail -c 38880 | \
        curl -sS --fail-with-body -X PUT -H 'Content-Type: application/octet-stream' \
        --data-binary @- http://{{ host }}/image

# The program's printf output; tio reconnects across reflashes.
monitor:
    tio $(ls /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null | head -1)

# Everything CI runs.
ci:
    nix fmt -- --ci
    just build
