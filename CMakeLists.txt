cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)

set(PICO_CXX_ENABLE_CXA_ATEXIT 1)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)


pico_sdk_init()

add_executable(elmio
    elmio.cpp
)

add_subdirectory(modules/pico-interfaces)
add_subdirectory(modules/pico-mcp23017)


pico_enable_stdio_usb(elmio 1)
pico_enable_stdio_uart(elmio 1)
pico_add_extra_outputs(elmio)

target_link_libraries(  
    elmio
    pico_stdlib
    pico_mcp23017
    hardware_dma
    hardware_pio
    hardware_irq
)