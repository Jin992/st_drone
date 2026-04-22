# Proposal: Flight Controller Base Structure

## What

Establish the foundational C module structure for the STM32F405 flight controller.
This creates the directory layout, CMake targets, interface contracts, and
communication protocol foundation that all future flight control features build on.

The structure is designed from day one for:
- **MAVLink** as the single communication protocol (telemetry + HITL)
- **HITL** via JSBSim on PC, connected directly to STM32 over UART or USB-CDC
- **Unit tests** for pure-logic modules (PID, mixer) compiled natively on host

## Why

The current codebase has three empty stub modules (`flight/`, `sensors/`, `comms/`).
Before writing any real flight logic the architecture needs to be set so that:

1. Flight algorithms are platform-agnostic C with no direct hardware calls
2. A HAL boundary allows swapping real sensors for JSBSim HIL data without
   changing any flight code
3. MAVLink is wired in early — it underpins both ground station telemetry and HITL

## Communication Architecture

MAVLink serves both roles in this project.

**Normal flight** (future — ESP32 WiFi bridge is out of scope for this change):

```
GCS (QGroundControl) ──WiFi── ESP32-C3 ──UART── STM32F405
```

**HITL mode** (in scope — direct cable connection):

```
JSBSim (PC)
    │ MAVLink over serial (UART or USB-CDC)
    │ — option A: USART1 (PA9/PA10) + USB-UART adapter
    │ — option B: USB OTG FS (PA11/PA12) as CDC virtual COM port
    ▼
STM32F405
    comms/ — same MAVLink layer as normal flight
    flight/ — same control code, unchanged
    hal/hitl/ — hal_imu_read() parses HIL_SENSOR
                hal_pwm_set() sends HIL_ACTUATOR_CONTROLS
```

No ESP32 involved in HITL. Direct cable from PC to STM32.
USB-CDC is preferred: single cable, no adapter, PA11/PA12 not used by anything else.

MAVLink HIL messages used:
- `HIL_SENSOR` (#107): JSBSim → STM32, synthetic IMU/baro/mag data
- `HIL_ACTUATOR_CONTROLS` (#93): STM32 → JSBSim, motor/servo commands
- `HEARTBEAT` (#0): both directions, connection health
- `ATTITUDE` (#30): STM32 → GCS, estimated attitude telemetry

## Module Architecture

```
┌──────────────────────────────────────────────────────┐
│                    src/main.c                        │
│     scheduler (1 kHz TIM6 IRQ), module init          │
├──────────────┬───────────────┬───────────────────────┤
│  flight/     │   sensors/    │   comms/              │
│  pid.c       │   imu.h       │   mavlink_tx.c        │
│  mixer.c     │   baro.h      │   mavlink_rx.c        │
│  attitude.c  │               │   hil.c               │
│              │               │                       │
│  ← pure C, no hardware deps ─────────────────────── │
├──────────────┴───────────────┴───────────────────────┤
│                    hal/hal.h                         │
│  hal_imu_read()  hal_pwm_set()  hal_time_us()        │
│  hal_uart_send() hal_uart_recv()                     │
├──────────────────────────┬───────────────────────────┤
│   hal/stm32/             │   hal/hitl/               │
│   Real SPI/I2C sensors   │   hal_imu_read() parses   │
│   Real TIM PWM outputs   │   HIL_SENSOR from UART    │
│                          │   hal_pwm_set() sends     │
│                          │   HIL_ACTUATOR_CONTROLS   │
└──────────────────────────┴───────────────────────────┘
```

Same flight code, same MAVLink layer, different HAL backend selected at compile time.

## Scope

### In scope

- `hal/` with `hal.h` interface + `stm32/` and `hitl/` backends
- `flight/` populated with skeleton files: pid, mixer, attitude estimator
- `sensors/` abstract interface headers (`imu.h`, `baro.h`)
- `comms/` MAVLink encode/decode + HIL message handling
- MAVLink C library added as dependency (git submodule or FetchContent from `mavlink/c_library_v2`)
- USB-CDC driver for STM32F405 (PA11/PA12) as the HITL physical link
- CMakePresets: add `hitl-debug` preset (ARM toolchain + `hal/hitl/` backend)
- `tests/` with native CMake target and unit tests for PID and mixer (pure math, no HAL)
- `src/main.c` updated to a 1 kHz control loop skeleton replacing the blink

### Out of scope

- Real sensor drivers (ICM-42688P SPI, BMP390) — separate change
- JSBSim PC-side configuration and launch scripts — separate change
- Full MAVLink telemetry set (ATTITUDE, GPS_RAW_INT, etc.) — separate change
- ESP32 MAVLink WiFi bridge firmware — separate change (after HITL works over USB)
- JSBSim PC-side launch scripts and model configuration — separate change
- PID / attitude filter tuning — requires flight testing
- DSHOT motor protocol — start with PWM

## Unit Test Strategy

Tests in `tests/` compile as a native x86 binary using a `host-debug` CMake preset.
They test only pure-logic modules (`flight/pid.c`, `flight/mixer.c`) — no HAL, no
hardware. Input→output math verification only.

Framework: **Unity** (fetched via FetchContent). Single-header, no dynamic allocation.

```bash
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

## File List

```
hal/
  hal.h                          ← interface: imu, pwm, uart, time
  stm32/hal_time.c               ← HAL_GetTick() wrapper
  stm32/hal_uart.c               ← USART2 to ESP32
  stm32/hal_imu.c                ← stub (real driver is separate change)
  hitl/hal_imu_hitl.c            ← parses MAVLink HIL_SENSOR from UART
  hitl/hal_pwm_hitl.c            ← sends MAVLink HIL_ACTUATOR_CONTROLS
  CMakeLists.txt

flight/
  pid.h / pid.c                  ← PID struct + update(dt, error) → output
  mixer.h / mixer.c              ← quad-X: 4x motor from roll/pitch/yaw/throttle
  attitude.h / attitude.c        ← Mahony filter skeleton
  CMakeLists.txt                 ← STATIC lib, zero platform headers

sensors/
  imu.h                          ← imu_data_t { ax, ay, az, gx, gy, gz }
  baro.h                         ← baro_data_t { pressure_pa, temp_c }
  CMakeLists.txt

comms/
  mavlink_tx.h / mavlink_tx.c    ← encode + send over HAL uart
  mavlink_rx.h / mavlink_rx.c    ← receive + parse from HAL uart
  hil.h / hil.c                  ← HIL_SENSOR → sensor structs, pwm → HIL_ACTUATOR
  CMakeLists.txt

tests/
  CMakeLists.txt                 ← FetchContent Unity, native toolchain
  test_pid.c                     ← step response, integrator windup
  test_mixer.c                   ← known roll/pitch/yaw → expected motor outputs

CMakePresets.json                ← add hitl-debug (ARM + hal/hitl), host-debug (native)
src/main.c                       ← HAL init + 1 kHz TIM6 IRQ control loop skeleton
```

## Success Criteria

- `cmake --preset debug && cmake --build --preset debug` produces flashable binary
- `cmake --preset hitl-debug && cmake --build --preset hitl-debug` compiles without error
- `cmake --preset host-debug && cmake --build --preset host-debug && ctest` — PID and mixer tests pass
- `flight/` contains zero STM32 or platform-specific `#include`s
- MAVLink library is available as a CMake target linkable by `comms/`
