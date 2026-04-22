#pragma once

#include <stdint.h>
#include <stddef.h>

void hal_usb_cdc_init(void);
int  hal_usb_cdc_send(const uint8_t *buf, size_t len);
int  hal_usb_cdc_recv(uint8_t *buf, size_t len);
void hal_usb_cdc_on_rx(const uint8_t *buf, uint16_t len);
