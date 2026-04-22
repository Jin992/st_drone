#include "hal/hal.h"
#include <string.h>

/* Stub — real SPI driver (ICM-42688P or similar) is a separate change. */
int hal_imu_read(imu_data_t *out)
{
    memset(out, 0, sizeof(*out));
    return 0;
}

int hal_baro_read(baro_data_t *out)
{
    memset(out, 0, sizeof(*out));
    return 0;
}
