#include "hal/hal.h"

/* UART to ESP32 is not used in HITL mode — all comms go through USB-CDC. */

size_t hal_uart_send(const uint8_t *buf, size_t len)
{
    (void)buf;
    (void)len;
    return 0;
}

size_t hal_uart_recv(uint8_t *buf, size_t len)
{
    (void)buf;
    (void)len;
    return 0;
}
