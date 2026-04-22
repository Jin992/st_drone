## ADDED Requirements

### Requirement: HAL interface isolates flight code from hardware
The project SHALL define a hardware abstraction layer in `hal/hal.h` declaring all
platform-dependent function signatures. Modules in `flight/`, `sensors/`, and `comms/`
SHALL NOT include any STM32-specific or platform-specific headers directly.

#### Scenario: Flight module has no platform headers
- **WHEN** a developer searches `flight/` for STM32 or platform-specific includes
- **THEN** no files under `flight/` contain `#include "stm32` or `#include "freertos`

#### Scenario: HAL interface is the only cross-boundary include
- **WHEN** flight code needs timing, UART, or sensor data
- **THEN** it calls functions declared in `hal/hal.h` only

### Requirement: Two HAL backends selectable at build time
The project SHALL provide two HAL backend directories: `hal/stm32/` implementing
real hardware access and `hal/hitl/` implementing HITL via MAVLink over USB-CDC.
The active backend SHALL be selected by the CMake preset, not by runtime flags.

#### Scenario: Normal build uses stm32 backend
- **WHEN** the developer builds with the `debug` preset
- **THEN** only `hal/stm32/` source files are compiled into the binary

#### Scenario: HITL build uses hitl backend
- **WHEN** the developer builds with the `hitl-debug` preset
- **THEN** only `hal/hitl/` source files are compiled; no `hal/stm32/` sources included

### Requirement: Control loop runs at 1 kHz via TIM6 interrupt
`src/main.c` SHALL configure TIM6 to fire at 1 kHz and run the flight control update
(sensor read → attitude estimate → PID → motor mix → actuator write) inside the ISR.

#### Scenario: Control loop executes at correct rate
- **WHEN** the firmware runs on STM32F405 at 168 MHz
- **THEN** the TIM6 ISR fires every 1 ms ± 1 µs

#### Scenario: Main loop is idle
- **WHEN** no interrupt is pending
- **THEN** `main()` executes only init and a low-priority idle loop (telemetry, parsing)

### Requirement: Module directory structure matches proposal
The repository SHALL contain the directories and skeleton source files defined in the
proposal: `flight/`, `sensors/`, `comms/`, `hal/stm32/`, `hal/hitl/`, `tests/`.

#### Scenario: All skeleton files present
- **WHEN** a developer clones the repository
- **THEN** `flight/pid.c`, `flight/mixer.c`, `flight/attitude.c`, `hal/hal.h`,
  `comms/mavlink_tx.c`, `comms/mavlink_rx.c`, and `tests/test_pid.c` all exist
