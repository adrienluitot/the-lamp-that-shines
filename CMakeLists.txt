cmake_minimum_required(VERSION 3.14)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(the-lamps-that-shines C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(libs)

add_executable(main
    main.c
)

pico_generate_pio_header(libs ${CMAKE_CURRENT_LIST_DIR}/dmx.pio)

# pull in common dependencies
target_link_libraries(main libs pico_stdlib hardware_pio hardware_pwm hardware_adc)
target_link_libraries(libs pico_stdlib hardware_pio hardware_adc)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# target_include_directories(main PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#                           "${PROJECT_SOURCE_DIR}/libs")

# create map/bin/hex file etc.
pico_add_extra_outputs(main)