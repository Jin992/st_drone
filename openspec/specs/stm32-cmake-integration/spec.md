## ADDED Requirements

### Requirement: stm32-cmake fetched via FetchContent
The build system SHALL fetch stm32-cmake (ObKo) using CMake `FetchContent` at configure time. No git submodules or manual downloads SHALL be required.

#### Scenario: First configure fetches stm32-cmake
- **WHEN** CMake is configured for the first time with no cache
- **THEN** stm32-cmake sources are downloaded and made available as CMake modules

#### Scenario: Subsequent configures use cached sources
- **WHEN** CMake is configured again after the first successful configure
- **THEN** no network access occurs and configure completes using the local FetchContent cache

### Requirement: STM32F405RGT6 HAL target is linked into firmware
The firmware target SHALL link against `stm32::f405rg` and `stm32::f4::hal` CMake targets provided by stm32-cmake, making STM32 HAL APIs available without manually specifying HAL source files.

#### Scenario: HAL headers are available in firmware sources
- **WHEN** `src/main.c` includes `stm32f4xx_hal.h`
- **THEN** compilation succeeds without missing header errors

### Requirement: Startup file and linker script provided by stm32-cmake
The build system SHALL use stm32-cmake helpers to attach the correct STM32F405 startup file and generate the linker script. No manually maintained linker script SHALL be required.

#### Scenario: Firmware ELF has correct memory layout
- **WHEN** the firmware is linked
- **THEN** the `.text` section is placed at Flash origin `0x08000000` and `.data`/`.bss` in SRAM at `0x20000000`

#### Scenario: Startup code initializes data and bss sections
- **WHEN** the firmware resets
- **THEN** `.data` is copied from Flash to SRAM and `.bss` is zeroed before `main()` is called
