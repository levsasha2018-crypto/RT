cmake_minimum_required(VERSION 3.15)
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)
include(pico_sdk_import.cmake)
project(231_firmware_template)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()
add_compile_options(-Wall)

add_executable(${PROJECT_NAME}
    main.c
    stdio-text-protocol/stdio-text-protocol.c
    api/api.c
    device/device.c
    logging/log.c
    led-task/led-task.c
    button-task/button-task.c
    pi-task/pi-task.c
    profiling/profiling.c
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} pico_stdlib pico_unique_id)
target_compile_definitions(${PROJECT_NAME} PRIVATE DEVICE_BOARD="${PICO_BOARD}")
if(DEFINED LOG_LEVEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LOG_LEVEL=${LOG_LEVEL})
endif()
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})
