cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)
# For clangd, which finds it in build/.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(inkpico C CXX ASM)
pico_sdk_init()

include($ENV{PICO_OTA_PATH}/pico_ota.cmake)

add_library(epd INTERFACE)
target_sources(epd INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/epd.c)
target_include_directories(epd INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(epd INTERFACE pico_stdlib hardware_spi)

# Each app is src/<app>.c, which tools/ota flashes as build/<app>.bin.
foreach(app image life)
    add_executable(${app} src/${app}.c)
    target_link_libraries(${app} epd)
    pico_enable_stdio_usb(${app} 1)
    pico_enable_stdio_uart(${app} 0)
    ota_app(${app})
endforeach()
