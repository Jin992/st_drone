#include "comms/mavlink_rx.h"

static mavlink_msg_handler_t s_handler;
static mavlink_message_t     s_msg;
static mavlink_status_t      s_status;

void mavlink_rx_set_handler(mavlink_msg_handler_t handler)
{
    s_handler = handler;
}

void mavlink_rx_feed(uint8_t byte)
{
    if (mavlink_parse_char(MAVLINK_COMM_0, byte, &s_msg, &s_status)) {
        if (s_handler)
            s_handler(&s_msg);
    }
}
