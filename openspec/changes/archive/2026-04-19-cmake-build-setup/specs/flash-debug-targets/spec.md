## ADDED Requirements

### Requirement: flash target programs firmware via OpenOCD
The build system SHALL provide a `flash` CMake custom target that invokes OpenOCD with `interface/stlink.cfg` and `target/stm32f4x.cfg` to program, verify, and reset the target board.

#### Scenario: flash target programs the board
- **WHEN** `cmake --build build/debug --target flash` is run with the board connected
- **THEN** OpenOCD programs the firmware ELF, verifies flash contents, and resets the MCU

#### Scenario: flash target depends on firmware build
- **WHEN** source files have changed and `--target flash` is invoked
- **THEN** the firmware is rebuilt before flashing

### Requirement: debug target starts OpenOCD GDB server
The build system SHALL provide a `debug` CMake custom target that starts an OpenOCD GDB server on port 3333, ready for `arm-none-eabi-gdb` to connect.

#### Scenario: debug target starts GDB server
- **WHEN** `cmake --build build/debug --target debug` is run
- **THEN** OpenOCD starts and listens on port 3333 for GDB connections

### Requirement: Build produces .elf, .bin, and .hex artifacts
The build system SHALL generate `.bin` and `.hex` files alongside the `.elf` as post-build steps using `arm-none-eabi-objcopy`.

#### Scenario: All three artifacts exist after build
- **WHEN** the firmware target is built successfully
- **THEN** `firmware.elf`, `firmware.bin`, and `firmware.hex` are all present in the build directory
