#pragma once

#include "common/mavlink.h"

/* Callback invoked when a complete MAVLink message is parsed. */
typedef void (*mavlink_msg_handler_t)(const mavlink_message_t *msg);

/* Register a handler called for every parsed message. */
void mavlink_rx_set_handler(mavlink_msg_handler_t handler);

/* Feed one byte from the UART stream into the parser.
   Calls the registered handler whenever a message is complete. */
void mavlink_rx_feed(uint8_t byte);
