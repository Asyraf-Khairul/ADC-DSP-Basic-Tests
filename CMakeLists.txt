# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

# The ARM compiler cannot link CMake's host-style test executable before the
# Pico SDK startup and syscall stubs are linked.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(adc_dsp_standalone C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

pico_sdk_init()

option(BENCH_SELF_TEST "Build synthetic DSP tests instead of live ADC firmware" OFF)
if(BENCH_SELF_TEST)
    set(BENCH_ENTRY tests/self_test.cpp)
else()
    set(BENCH_ENTRY main.cpp)
endif()
add_executable(adc_dsp_standalone
    ${BENCH_ENTRY}
    bench.cpp
    settings.cpp
    front_panel.cpp
    controls.cpp
    stream.cpp
    display.cpp
    ssd1306.c
    receiver_dsp/rx_dsp.cpp
    receiver_dsp/fft.cpp
    receiver_dsp/fft_filter.cpp
    receiver_dsp/utils.cpp
    receiver_dsp/cic_corrections.cpp
    receiver_dsp/noise_reduction.cpp
    receiver_dsp/ring_buffer_lib.c
)
target_include_directories(adc_dsp_standalone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(adc_dsp_standalone PRIVATE
    pico_stdlib
    pico_multicore
    pico_util
    hardware_adc
    hardware_dma
    hardware_i2c
    hardware_pwm
    hardware_clocks
    hardware_pio
    m
)

target_compile_options(adc_dsp_standalone PRIVATE -fwrapv)
target_compile_definitions(adc_dsp_standalone PRIVATE PICO_STACK_SIZE=0x1000 PICO_CORE1_STACK_SIZE=0x800)
pico_enable_stdio_usb(adc_dsp_standalone 1)
pico_enable_stdio_uart(adc_dsp_standalone 0)
pico_add_extra_outputs(adc_dsp_standalone)

