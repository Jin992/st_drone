#include "hal/hal.h"

/* Stub — real PWM/DSHOT driver is a separate change. */
void hal_pwm_set(const float throttle[4])
{
    (void)throttle;
}
