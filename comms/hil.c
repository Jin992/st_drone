#include "comms/hil.h"

void hil_handle_sensor(const mavlink_message_t *msg, imu_data_t *out)
{
    if (msg->msgid != MAVLINK_MSG_ID_HIL_SENSOR)
        return;

    mavlink_hil_sensor_t pkt;
    mavlink_msg_hil_sensor_decode(msg, &pkt);

    out->ax = pkt.xacc;
    out->ay = pkt.yacc;
    out->az = pkt.zacc;
    out->gx = pkt.xgyro;
    out->gy = pkt.ygyro;
    out->gz = pkt.zgyro;
    out->mx = pkt.xmag;
    out->my = pkt.ymag;
    out->mz = pkt.zmag;
    out->temperature = pkt.temperature;
}
