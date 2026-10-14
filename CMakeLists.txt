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
option(BENCH_OPERATION_BENCHMARK "Build isolated BASIC operation benchmark" OFF)
if(BENCH_SELF_TEST AND BENCH_OPERATION_BENCHMARK)
    message(FATAL_ERROR "Select only one synthetic test entry point")
endif()
if(BENCH_OPERATION_BENCHMARK)
    set(BENCH_ENTRY tests/basic_operation_benchmark.cpp)
elseif(BENCH_SELF_TEST)
    set(BENCH_ENTRY tests/self_test.cpp)
else()
    set(BENCH_ENTRY main.cpp)
endif()
add_executable(adc_dsp_standalone
    ${BENCH_ENTRY}
    bench.cpp
    basic_dsp.cpp
    input_statistics.cpp
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
    receiver_dsp/spectrum_log.cpp
    receiver_dsp/ring_buffer_lib.c
)
target_include_directories(adc_dsp_standalone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set(BENCH_BUILD_LABEL "am-profile-v1" CACHE STRING "Human-readable firmware build label")
file(GLOB BENCH_ID_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/receiver_dsp/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/receiver_dsp/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/receiver_dsp/*.h")
list(APPEND BENCH_ID_FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" "${CMAKE_CURRENT_SOURCE_DIR}/build_identity.h.in")
list(SORT BENCH_ID_FILES)
set(BENCH_ID_CONTENT "${BENCH_BUILD_LABEL};${CMAKE_BUILD_TYPE};${BENCH_SELF_TEST};${BENCH_OPERATION_BENCHMARK}")
foreach(BENCH_ID_FILE IN LISTS BENCH_ID_FILES)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${BENCH_ID_FILE}")
    file(SHA256 "${BENCH_ID_FILE}" BENCH_FILE_SHA)
    get_filename_component(BENCH_FILE_NAME "${BENCH_ID_FILE}" NAME)
    string(APPEND BENCH_ID_CONTENT ";${BENCH_FILE_NAME}:${BENCH_FILE_SHA}")
endforeach()
string(SHA256 BENCH_SOURCE_SHA256 "${BENCH_ID_CONTENT}")
execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE BENCH_BASE_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file(build_identity.h.in build_identity.h @ONLY)
target_include_directories(adc_dsp_standalone PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

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

