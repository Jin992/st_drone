## 1. MAVLink Library

- [x] 1.1 Add `mavlink/c_library_v2` via FetchContent in root `CMakeLists.txt`, pinned to a specific commit
- [x] 1.2 Create a `MAVLINK` CMake INTERFACE target exposing the `common` dialect headers
- [x] 1.3 Verify `#include "common/mavlink.h"` compiles in a minimal test target

## 2. HAL Interface

- [x] 2.1 Create `hal/hal.h` declaring: `hal_imu_read()`, `hal_pwm_set()`, `hal_time_us()`, `hal_uart_send()`, `hal_uart_recv()`
- [x] 2.2 Define shared types in `hal/hal.h`: `imu_data_t`, `baro_data_t`
- [x] 2.3 Create `hal/CMakeLists.txt` with two STATIC library targets: `hal_stm32` and `hal_hitl`

## 3. STM32 HAL Backend

- [x] 3.1 Create `hal/stm32/hal_time.c` implementing `hal_time_us()` via `HAL_GetTick()`
- [x] 3.2 Create `hal/stm32/hal_uart.c` implementing `hal_uart_send/recv()` on USART2 (to ESP32)
- [x] 3.3 Create `hal/stm32/hal_imu.c` with a stub `hal_imu_read()` returning zeroed `imu_data_t` (real driver is a separate change)
- [x] 3.4 Create `hal/stm32/hal_pwm.c` with a stub `hal_pwm_set()` (real PWM driver is a separate change)

## 4. HITL HAL Backend

- [x] 4.1 Create `hal/hitl/hal_usb_cdc.c` initialising USB-CDC on PA11/PA12 using STM32CubeF4 USB middleware
- [x] 4.2 Create `hal/hitl/hal_imu_hitl.c` implementing `hal_imu_read()`: parse `HIL_SENSOR` (#107) from USB-CDC ring buffer
- [x] 4.3 Create `hal/hitl/hal_pwm_hitl.c` implementing `hal_pwm_set()`: encode and send `HIL_ACTUATOR_CONTROLS` (#93) over USB-CDC
- [x] 4.4 Create `hal/hitl/hal_uart_hitl.c` with no-op stubs for UART (not used in HITL mode)

## 5. Flight Module Skeletons

- [x] 5.1 Create `flight/pid.h` / `flight/pid.c`: `pid_t` struct with Kp/Ki/Kd/limit fields, `pid_update(pid_t*, float dt, float error)` returning float
- [x] 5.2 Create `flight/mixer.h` / `flight/mixer.c`: `mixer_update(float throttle, float roll, float pitch, float yaw, float out[4])` for quad-X
- [x] 5.3 Create `flight/attitude.h` / `flight/attitude.c`: Mahony filter skeleton with `attitude_update(imu_data_t*, float dt)` and `attitude_get_euler(float* roll, float* pitch, float* yaw)`
- [x] 5.4 Update `flight/CMakeLists.txt` to compile `pid.c`, `mixer.c`, `attitude.c` as a STATIC library with no platform includes

## 6. Sensors and Comms Skeletons

- [x] 6.1 Create `sensors/imu.h` and `sensors/baro.h` with data structs (types shared with `hal/hal.h`)
- [x] 6.2 Create `comms/mavlink_tx.h` / `comms/mavlink_tx.c`: `mavlink_send_attitude()` skeleton
- [x] 6.3 Create `comms/mavlink_rx.h` / `comms/mavlink_rx.c`: byte-by-byte MAVLink parser feeding a message callback
- [x] 6.4 Create `comms/hil.h` / `comms/hil.c`: `hil_handle_sensor()` converting `HIL_SENSOR` fields to `imu_data_t`
- [x] 6.5 Update `comms/CMakeLists.txt` to compile these files and link the `MAVLINK` target

## 7. Main Entry Point

- [x] 7.1 Update `src/main.c`: HAL init, TIM6 configured for 1 kHz IRQ
- [x] 7.2 Implement `TIM6_DAC_IRQHandler`: call `hal_imu_read()` → `attitude_update()` → PID stubs → `mixer_update()` → `hal_pwm_set()`
- [x] 7.3 Add overrun detection: increment a counter if previous ISR is still running when next fires

## 8. CMake Presets

- [x] 8.1 Add `hitl-debug` preset to `CMakePresets.json`: ARM toolchain, `hal/hitl/` backend, `HAL_BACKEND=hitl` cache variable
- [x] 8.2 Add `host-debug` preset: native toolchain (no cross-compile), links only `flight/` and Unity, skips STM32 HAL fetch
- [x] 8.3 Verify `cmake --preset debug` still builds the original target without errors
- [x] 8.4 Verify `cmake --preset hitl-debug` compiles without errors

## 9. Unit Tests

- [x] 9.1 Create `tests/CMakeLists.txt`: FetchContent Unity, native executable target `drone_tests`
- [x] 9.2 Create `tests/test_pid.c`: proportional step response test, integrator windup clamp test
- [x] 9.3 Create `tests/test_mixer.c`: pure throttle equal outputs test, roll differential test
- [x] 9.4 Register tests with CTest, verify `ctest --preset host-debug` reports all passing
