cmake_minimum_required(VERSION 3.21)

include(FetchContent)

FetchContent_Declare(
    stm32-cmake
    GIT_REPOSITORY https://github.com/ObKo/stm32-cmake.git
    GIT_TAG        v2.1.0
)
FetchContent_MakeAvailable(stm32-cmake)

# stm32-cmake is not used as the CMake toolchain here (Conan handles cross-compile).
# We include its modules manually to get find_package(CMSIS/HAL) and helper functions.
list(APPEND CMAKE_MODULE_PATH "${stm32-cmake_SOURCE_DIR}/cmake")
set(STM32_CMAKE_DIR "${stm32-cmake_SOURCE_DIR}/cmake")
include(stm32/common)
include(stm32/utilities)

project(drone C ASM)

# Download STM32CubeF4 CMSIS and HAL sources via FetchContent (modern API avoids
# deprecated FetchContent_Populate calls inside stm32_fetch_cmsis/stm32_fetch_hal)
FetchContent_MakeAvailable(STM32-CMSIS STM32-CMSIS-F4 STM32-HAL-F4)
set(STM32_CMSIS_PATH    ${stm32-cmsis_SOURCE_DIR})
set(STM32_CMSIS_F4_PATH ${stm32-cmsis-f4_SOURCE_DIR})
set(STM32_HAL_F4_PATH   ${stm32-hal-f4_SOURCE_DIR})

find_package(CMSIS COMPONENTS STM32F405RG REQUIRED)
find_package(HAL   COMPONENTS STM32F4 RCC GPIO CORTEX UART TIM PCD REQUIRED)

# MAVLink C library v2 — header-only, no CMakeLists.txt in the repo.
FetchContent_Declare(
    mavlink
    GIT_REPOSITORY https://github.com/mavlink/c_library_v2.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(mavlink)
add_library(MAVLINK INTERFACE)
target_include_directories(MAVLINK INTERFACE ${mavlink_SOURCE_DIR})

set(HAL_BACKEND "stm32" CACHE STRING "HAL backend: stm32 or hitl")
set_property(CACHE HAL_BACKEND PROPERTY STRINGS stm32 hitl)

if(HAL_BACKEND STREQUAL "hitl")
    FetchContent_Declare(
        stm32-usb-device
        GIT_REPOSITORY https://github.com/STMicroelectronics/stm32_mw_usb_device.git
        GIT_TAG        v2.9.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(stm32-usb-device)

    add_library(USB_DEVICE STATIC
        ${stm32-usb-device_SOURCE_DIR}/Core/Src/usbd_core.c
        ${stm32-usb-device_SOURCE_DIR}/Core/Src/usbd_ctlreq.c
        ${stm32-usb-device_SOURCE_DIR}/Core/Src/usbd_ioreq.c
        ${stm32-usb-device_SOURCE_DIR}/Class/CDC/Src/usbd_cdc.c
        ${STM32_HAL_F4_PATH}/Src/stm32f4xx_ll_usb.c
        ${CMAKE_SOURCE_DIR}/hal/hitl/usb/usbd_conf.c
    )
    target_include_directories(USB_DEVICE PUBLIC
        ${stm32-usb-device_SOURCE_DIR}/Core/Inc
        ${stm32-usb-device_SOURCE_DIR}/Class/CDC/Inc
        ${CMAKE_SOURCE_DIR}/hal/hitl/usb
        ${CMAKE_SOURCE_DIR}/src
    )
    target_link_libraries(USB_DEVICE PUBLIC
        CMSIS::STM32::F405RG
        HAL::STM32::F4::PCD
        HAL::STM32::F4::PCDEx
    )
endif()

add_subdirectory(hal)
add_subdirectory(flight)
add_subdirectory(sensors)
add_subdirectory(comms)

add_executable(drone src/main.c)

target_include_directories(drone PRIVATE src)

if(HAL_BACKEND STREQUAL "hitl")
    set(HAL_TARGET hal_hitl)
else()
    set(HAL_TARGET hal_stm32)
endif()

target_link_libraries(drone PRIVATE
    CMSIS::STM32::F405RG
    HAL::STM32::F4::RCC
    HAL::STM32::F4::GPIO
    HAL::STM32::F4::CORTEX
    HAL::STM32::F4::UART
    HAL::STM32::F4::TIM
    STM32::NoSys
    -Wl,--start-group
    ${HAL_TARGET}
    flight
    sensors
    comms
    -Wl,--end-group
)

target_compile_options(drone PRIVATE
    -Wall
    -fdata-sections
    -ffunction-sections
)

target_link_options(drone PRIVATE
    -Wl,--gc-sections
    -Wl,--print-memory-usage
)

stm32_generate_binary_file(drone)
stm32_generate_hex_file(drone)

add_custom_target(flash
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
            -c "program $<TARGET_FILE:drone> verify reset exit"
    DEPENDS drone
    COMMENT "Flashing firmware via OpenOCD"
)

add_custom_target(debug
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
    DEPENDS drone
    COMMENT "Starting OpenOCD GDB server on :3333"
)
