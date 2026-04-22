#pragma once

#include <stdint.h>
#include "flight/attitude.h"

/* Send a MAVLINK ATTITUDE (#30) message from current filter state.
   time_boot_ms — milliseconds since boot (caller provides from HAL). */
void mavlink_send_attitude(const attitude_t *att, uint32_t time_boot_ms);

/* Send a MAVLINK HEARTBEAT (#0) message. */
void mavlink_send_heartbeat(void);
