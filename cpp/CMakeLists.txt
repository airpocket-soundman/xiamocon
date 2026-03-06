set(LIB_NAME libxmc)
set(PFM_COMMON_SRC_DIR platform/common/src)
set(PFM_COMMON_INC_DIR platform/common/include)
set(PFM_BOARD_SRC_DIR platform/rp2350_pico_sdk/src)
set(PFM_BOARD_INC_DIR platform/rp2350_pico_sdk/include)
set(LIB_SRC_DIR library/src)
set(LIB_INC_DIR library/include)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(${LIB_NAME} C CXX ASM)

file(GLOB CPP_FILES
    ${PFM_COMMON_SRC_DIR}/*.cpp
    ${PFM_COMMON_SRC_DIR}/*.c
    ${PFM_BOARD_SRC_DIR}/*.cpp
    ${PFM_BOARD_SRC_DIR}/*.c
    ${LIB_SRC_DIR}/*.cpp
    ${LIB_SRC_DIR}/*.c
    ${LIB_SRC_DIR}/**/*.cpp
    ${LIB_SRC_DIR}/**/*.c
)

add_library(${LIB_NAME} STATIC
    ${CPP_FILES}
)

target_compile_options(${LIB_NAME} PUBLIC -O2 -Wall)
target_compile_features(${LIB_NAME} PUBLIC cxx_std_17)

target_include_directories(${LIB_NAME} PUBLIC
    ${PFM_COMMON_INC_DIR}
    ${PFM_BOARD_INC_DIR}
    ${LIB_INC_DIR}
)

target_link_libraries(
    ${LIB_NAME}
    pico_stdlib
    pico_multicore
    pico_stdio_usb
    hardware_dma
    hardware_spi
    hardware_i2c
    hardware_powman
    hardware_vreg
    hardware_watchdog
    hardware_sleep
)
