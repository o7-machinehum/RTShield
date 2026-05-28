cmake_minimum_required(VERSION 3.20)

set(PICO_PLATFORM rp2350 CACHE STRING "Pico platform")
set(PICO_BOARD rtshield_rp2350b CACHE STRING "Pico board")
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards CACHE STRING "Local Pico board headers")
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../third_party/pico-sdk CACHE PATH "Path to the Pico SDK")

get_filename_component(PICO_SDK_PATH "${PICO_SDK_PATH}" ABSOLUTE)
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(rtshield_firmware C CXX ASM)
pico_sdk_init()

add_executable(rtshield_fw main.c)
target_link_libraries(rtshield_fw pico_stdlib hardware_adc hardware_i2c hardware_uart)
target_compile_options(rtshield_fw PRIVATE -Wall -Wextra)

pico_enable_stdio_usb(rtshield_fw 0)
pico_enable_stdio_uart(rtshield_fw 0)
pico_add_extra_outputs(rtshield_fw)
