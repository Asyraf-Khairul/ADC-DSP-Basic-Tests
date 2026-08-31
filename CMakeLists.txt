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

pico_sdk_init()

add_executable(adc_dsp_standalone
    main.cpp
    adc.cpp
    dsp.cpp
    display.cpp
    pwm.cpp
    ssd1306.c
    nco.cpp
    clocks_pico.cpp
)

target_compile_features(adc_dsp_standalone PRIVATE cxx_std_17)

target_link_libraries(adc_dsp_standalone PRIVATE
    pico_stdlib
    hardware_adc
    hardware_dma
    hardware_i2c
    hardware_pwm
    hardware_clocks
    hardware_pio
    m
)

pico_enable_stdio_usb(adc_dsp_standalone 0)
pico_enable_stdio_uart(adc_dsp_standalone 0)
pico_add_extra_outputs(adc_dsp_standalone)

