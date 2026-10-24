build:
    cmake -S . -B build -G Ninja
    ninja -C build

# Without BOOTSEL: -f asks the running program to reboot into the bootloader over USB.
flash: build
    picotool load -f -x build/inkpico.uf2

# The program's printf output; tio reconnects across reflashes.
monitor:
    tio $(ls /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null | head -1)

# Everything CI runs.
ci:
    nix fmt -- --ci
    just build
