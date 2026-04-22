#include "hal/hal.h"
#include "stm32f4xx_hal.h"

uint32_t hal_time_us(void)
{
    /* HAL_GetTick() returns milliseconds; multiply for microseconds.
       For higher resolution, a free-running timer should be used instead. */
    return HAL_GetTick() * 1000u;
}
