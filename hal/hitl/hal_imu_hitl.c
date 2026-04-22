#include "hal/hal.h"
#include "hal/hitl/hal_usb_cdc.h"
#include "common/mavlink.h"
#include <string.h>

static imu_data_t  s_last;
static uint32_t    s_last_tick;
static int         s_has_data;

/* Call from main loop (not ISR) to drain USB-CDC and parse MAVLink bytes. */
void hal_hitl_poll(void)
{
    static mavlink_message_t msg;
    static mavlink_status_t  status;

    uint8_t byte;
    while (hal_usb_cdc_recv(&byte, 1) == 1) {
        if (mavlink_parse_char(MAVLINK_COMM_0, byte, &msg, &status)) {
            if (msg.msgid == MAVLINK_MSG_ID_HIL_SENSOR) {
                mavlink_hil_sensor_t pkt;
                mavlink_msg_hil_sensor_decode(&msg, &pkt);
                s_last.ax = pkt.xacc;
                s_last.ay = pkt.yacc;
                s_last.az = pkt.zacc;
                s_last.gx = pkt.xgyro;
                s_last.gy = pkt.ygyro;
                s_last.gz = pkt.zgyro;
                s_last.mx = pkt.xmag;
                s_last.my = pkt.ymag;
                s_last.mz = pkt.zmag;
                s_last.temperature = pkt.temperature;
                s_last_tick = HAL_GetTick();
                s_has_data  = 1;
            }
        }
    }
}

int hal_imu_read(imu_data_t *out)
{
    if (!s_has_data) {
        memset(out, 0, sizeof(*out));
        return -1;
    }
    *out = s_last;
    /* Stale if no new packet for 100 ms. */
    return (HAL_GetTick() - s_last_tick < 100) ? 0 : -1;
}

int hal_baro_read(baro_data_t *out)
{
    memset(out, 0, sizeof(*out));
    return -1;  /* barometer via HIL_SENSOR is a follow-up */
}
