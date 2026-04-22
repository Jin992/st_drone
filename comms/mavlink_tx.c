#include "comms/mavlink_tx.h"
#include "hal/hal.h"
#include "common/mavlink.h"

#define FC_SYSID  1
#define FC_COMPID 200

static void send_buf(const uint8_t *buf, uint16_t len)
{
    hal_uart_send(buf, len);
}

void mavlink_send_heartbeat(void)
{
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_heartbeat_pack(FC_SYSID, FC_COMPID, &msg,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_GENERIC,
        0, 0, MAV_STATE_STANDBY);
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    send_buf(buf, len);
}

void mavlink_send_attitude(const attitude_t *att, uint32_t time_boot_ms)
{
    float roll, pitch, yaw;
    attitude_get_euler(att, &roll, &pitch, &yaw);

    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_attitude_pack(FC_SYSID, FC_COMPID, &msg,
        time_boot_ms,
        roll, pitch, yaw,
        0.0f, 0.0f, 0.0f);
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    send_buf(buf, len);
}
