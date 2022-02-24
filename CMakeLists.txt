cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(the-lamps-that-shines C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(main
    main.c
)

pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/full_speed.pio)

# pull in common dependencies
target_link_libraries(main pico_stdlib hardware_pio)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)