#include "hal/hal.h"
#include "hal/hitl/hal_usb_cdc.h"
#include "common/mavlink.h"

void hal_pwm_set(const float throttle[4])
{
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    mavlink_hil_actuator_controls_t pkt = {0};
    pkt.controls[0] = throttle[0];
    pkt.controls[1] = throttle[1];
    pkt.controls[2] = throttle[2];
    pkt.controls[3] = throttle[3];
    /* controls[4..15] remain 0 (unused) */
    pkt.mode = 0;  /* MAV_MODE_FLAG_SAFETY_ARMED when armed */

    uint16_t len = mavlink_msg_hil_actuator_controls_encode(
        1,   /* system id  */
        200, /* component id — flight controller */
        &msg, &pkt);
    len = mavlink_msg_to_send_buffer(buf, &msg);
    hal_usb_cdc_send(buf, len);
}
