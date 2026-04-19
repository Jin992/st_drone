## ADDED Requirements

### Requirement: Toolchain file configures bare-metal cross-compilation
The build system SHALL use `cmake/arm-none-eabi.cmake` as the CMake toolchain file, configuring `arm-none-eabi-gcc` as the C compiler, setting `CMAKE_SYSTEM_NAME` to `Generic`, and applying Cortex-M4 + FPU flags (`-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb`) to all targets.

#### Scenario: CMake configures without host compiler errors
- **WHEN** CMake is configured with the arm-none-eabi toolchain file
- **THEN** configuration succeeds without errors about missing host libraries or failed linker checks

#### Scenario: Compiled object targets Cortex-M4
- **WHEN** a `.c` file is compiled
- **THEN** the resulting object file is ELF for ARM, Thumb instruction set, hard-float ABI

### Requirement: CMakePresets.json provides debug and release presets
The project SHALL provide a `CMakePresets.json` with a hidden `base` preset and two inheriting presets (`debug`, `release`) compatible with VS Code CMake Tools extension.

#### Scenario: VS Code CMake Tools lists presets
- **WHEN** the project is opened in VS Code with CMake Tools installed
- **THEN** the extension shows `debug` and `release` as selectable configure presets

#### Scenario: Debug preset builds to build/debug
- **WHEN** the `debug` preset is selected and configured
- **THEN** CMake generates build files in `build/debug/` with `-O0 -g3`

#### Scenario: Release preset builds to build/release
- **WHEN** the `release` preset is selected and configured
- **THEN** CMake generates build files in `build/release/` with `-O2`

### Requirement: compile_commands.json is always generated
The build system SHALL set `CMAKE_EXPORT_COMPILE_COMMANDS=ON` in all presets so IntelliSense works correctly in VS Code with the cross-compiler include paths.

#### Scenario: compile_commands.json exists after configure
- **WHEN** any preset is configured
- **THEN** `compile_commands.json` is present in the build directory
