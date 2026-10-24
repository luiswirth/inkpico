cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)
# For clangd, which finds it in build/.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(inkpico C CXX ASM)
pico_sdk_init()

add_executable(inkpico src/main.c src/epd.c)
target_link_libraries(inkpico pico_stdlib hardware_spi pico_cyw43_arch_none)
pico_enable_stdio_usb(inkpico 1)
pico_enable_stdio_uart(inkpico 0)
pico_add_extra_outputs(inkpico)
