## ADDED Requirements

### Requirement: main.c blinks PC13 LED using STM32 HAL
The firmware SHALL contain a `src/main.c` that initializes the HAL, configures PC13 as a GPIO output, and toggles it in an infinite loop with a delay, using STM32 HAL APIs.

#### Scenario: LED blinks on hardware
- **WHEN** the firmware is flashed to the WeAct STM32F405RGT6
- **THEN** the blue LED on PC13 blinks at a visible rate (~1 Hz)

#### Scenario: main.c compiles cleanly
- **WHEN** the project is built with the debug preset
- **THEN** `src/main.c` compiles with no errors or warnings

### Requirement: Subsystem directories scaffold without breaking the build
The project SHALL include stub `CMakeLists.txt` files in `flight/`, `sensors/`, and `comms/` that define empty static libraries. The root build SHALL compile and link successfully even when these libraries contain no source files.

#### Scenario: Empty subsystem libraries link without errors
- **WHEN** the project is built from scratch
- **THEN** the firmware ELF is produced without linker errors related to empty subsystem libraries
