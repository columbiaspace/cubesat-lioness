cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
add_executable(blink
  blink.c
)

target_include_directories(blink PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Uncomment if you want to flash your board via picotool.
# pico_enable_stdio_usb(blink 1)
pico_add_extra_outputs(blink)

target_link_libraries(blink
  pico_stdlib
)

target_link_libraries(pico_stdlib)