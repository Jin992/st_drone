set(HAL_BACKEND "stm32" CACHE STRING "HAL backend: stm32 or hitl")
set_property(CACHE HAL_BACKEND PROPERTY STRINGS stm32 hitl)
message(STATUS "HAL backend: ${HAL_BACKEND}")

# ── STM32 backend ────────────────────────────────────────────────────────
add_library(hal_stm32 STATIC
    stm32/hal_init.c
    stm32/hal_time.c
    stm32/hal_uart.c
    stm32/hal_imu.c
    stm32/hal_pwm.c
)
target_include_directories(hal_stm32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(hal_stm32 PUBLIC
    sensors
    CMSIS::STM32::F405RG
    HAL::STM32::F4::RCC
    HAL::STM32::F4::GPIO
    HAL::STM32::F4::CORTEX
    HAL::STM32::F4::UART
    HAL::STM32::F4::TIM
    STM32::NoSys
)

# ── HITL backend ─────────────────────────────────────────────────────────
add_library(hal_hitl STATIC
    hitl/hal_usb_cdc.c
    hitl/usb/usbd_desc.c
    hitl/usb/usbd_cdc_if.c
    hitl/hal_imu_hitl.c
    hitl/hal_pwm_hitl.c
    hitl/hal_uart_hitl.c
    hitl/hal_time_hitl.c
)
target_include_directories(hal_hitl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(hal_hitl PUBLIC
    sensors
    MAVLINK
    USB_DEVICE
    CMSIS::STM32::F405RG
    HAL::STM32::F4::RCC
    HAL::STM32::F4::GPIO
    HAL::STM32::F4::CORTEX
    HAL::STM32::F4::TIM
    STM32::NoSys
)
