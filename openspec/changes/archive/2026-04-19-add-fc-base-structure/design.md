## Context

The STM32F405 project currently has three empty stub modules (`flight/`, `sensors/`, `comms/`) and a blink `main.c`. Before any flight logic is written, the architectural skeleton needs to be in place. The key constraint is that the same flight control code must run on real hardware (normal flight) and with simulated sensor data (HITL via JSBSim), without modification. A second constraint is testability: pure-logic modules must be verifiable on a dev machine without flashing hardware.

## Goals / Non-Goals

**Goals:**
- Define a HAL boundary that isolates flight code from hardware
- Wire MAVLink as the single communication protocol for both telemetry and HITL
- Enable HITL via JSBSim over USB-CDC with no code changes to flight algorithms
- Enable unit tests for `flight/` compiled natively (no cross-compiler, no hardware)
- Replace the blink `main.c` with a 1 kHz control loop skeleton

**Non-Goals:**
- Real sensor drivers (ICM-42688P, BMP390) — separate change
- JSBSim PC-side model and launch scripts — separate change
- ESP32 WiFi bridge — separate change
- Full MAVLink telemetry set (GPS, battery, etc.) — separate change
- PID / filter tuning — requires flight hardware

## Decisions

### HAL backend selection: compile-time CMake targets, not runtime flags

Two backends exist: `hal/stm32/` (real hardware) and `hal/hitl/` (USB-CDC + MAVLink HIL).
The backend is selected by which CMake preset is used — `debug` links `hal/stm32/`,
`hitl-debug` links `hal/hitl/`. No runtime branching, no `#ifdef HAL_BACKEND` in flight code.

**Alternative considered:** single source with `#ifdef HITL` guards. Rejected — guards
leak platform knowledge into flight code and make unit testing harder.

### HITL physical link: USB-CDC (PA11/PA12)

STM32F405 has USB OTG FS on PA11/PA12, unused by any other subsystem. USB-CDC creates
a virtual COM port on the PC — JSBSim connects to it as a serial port. No adapter cable
needed, single USB-C connection to the WeAct board.

**Alternative considered:** USART1 (PA9/PA10) + USB-UART adapter. Rejected — requires
extra hardware and the USB-C port is already on the board.

### MAVLink library: FetchContent from `mavlink/c_library_v2`

Pure generated C headers, no malloc, no source files to compile. Added via CMake
`FetchContent_Declare` pointing to the official Mavlink GitHub repo at a pinned commit.
Available as a header-only CMake INTERFACE target.

**Alternative considered:** git submodule. Will migrate to submodule if offline builds
become important. For now FetchContent is simpler.

### Control loop: bare-metal TIM6 IRQ at 1 kHz

TIM6 triggers the 1 kHz control loop interrupt. `main.c` contains only init and an
idle loop; all flight logic runs inside the ISR. No RTOS for now.

**Alternative considered:** FreeRTOS task at 1 kHz. Adds scheduling overhead and
complexity not justified by current requirements. Can be added later.

### Unit test framework: Unity via FetchContent

Unity is a single-file C test framework with no dynamic allocation, widely used in
embedded projects. Tests compile as a native x86 binary via a `host-debug` CMake
preset. Only `flight/` (pure logic) is tested this way; HAL-dependent code is not.

**Alternative considered:** Google Test (C++). Rejected — this is a C project and
introducing a C++ test framework adds unnecessary complexity.

### Attitude estimator: Mahony filter

Mahony is a complementary filter with integral correction, well-documented for
quadrotor use, low compute requirements (fits 1 kHz loop on F405 with headroom).
Skeleton only in this change — coefficients tuned separately.

**Alternative considered:** Madgwick filter. Similar complexity, slightly different
convergence characteristics. Either works; Mahony chosen for wider reference material.

## Risks / Trade-offs

- **USB-CDC init complexity** → STM32 USB middleware (STM32_USB_Device_Library) adds
  ~4 kB flash and requires correct clock configuration. Mitigation: use STM32CubeF4
  USB CDC example as reference; init is well-understood.

- **1 kHz ISR timing jitter** → if ISR takes >1 ms, loop overruns. Mitigation: keep
  ISR lean (read sensors, run PID, write outputs); log overruns via a counter.

- **MAVLink parsing in ISR** → parsing in interrupt context risks blocking. Mitigation:
  receive bytes into a ring buffer from UART/USB DMA; parse in main loop or a lower-
  priority task; feed parsed structs to ISR via volatile shared memory.

- **FetchContent network dependency** → build fails without internet on first run.
  Mitigation: document requirement; migrate to submodule if offline builds needed.

## Open Questions

- Which MAVLink dialect to use? `common` covers HIL messages; `ardupilotmega` adds
  extra telemetry. Start with `common` — can extend later.
- Does JSBSim need a custom FDM model for this airframe, or is a generic quadrotor
  model sufficient for initial HITL testing? (Out of scope for this change.)
