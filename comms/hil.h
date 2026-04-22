#pragma once

#include "common/mavlink.h"
#include "sensors/imu.h"

/* Convert a parsed HIL_SENSOR message into an imu_data_t struct. */
void hil_handle_sensor(const mavlink_message_t *msg, imu_data_t *out);
