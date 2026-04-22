#pragma once

#include <stdint.h>
#include <stddef.h>
#include "sensors/imu.h"
#include "sensors/baro.h"

/* ── Platform init ──────────────────────────────────────────────────────── */

/* Initialise all hardware peripherals required by this backend (clocks,
   GPIO, UART, USB-CDC, etc.).  Must be called once before any other hal_*. */
void hal_init(void);

/* ── Time ───────────────────────────────────────────────────────────────── */

/* Microseconds since boot (wraps at ~4294 s). */
uint32_t hal_time_us(void);

/* ── Sensors ────────────────────────────────────────────────────────────── */

/* Fill *out with the latest IMU sample.
   Returns 0 on success, -1 if data is stale (>100 ms old). */
int hal_imu_read(imu_data_t *out);

/* Fill *out with the latest barometer sample.
   Returns 0 on success, -1 if data is stale. */
int hal_baro_read(baro_data_t *out);

/* ── Actuators ──────────────────────────────────────────────────────────── */

/* Set motor throttle values (4 motors, quad-X layout).
   Each value is in [0.0, 1.0]. */
void hal_pwm_set(const float throttle[4]);

/* ── UART (ESP32 bridge) ─────────────────────────────────────────────────── */

/* Send len bytes from buf over the ESP32 UART bridge.
   Returns number of bytes actually sent. */
size_t hal_uart_send(const uint8_t *buf, size_t len);

/* Read up to len bytes from the ESP32 UART bridge into buf.
   Returns number of bytes actually read (may be 0). */
size_t hal_uart_recv(uint8_t *buf, size_t len);
