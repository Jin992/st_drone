## ADDED Requirements

### Requirement: conanfile.py declares project dependencies
The project SHALL provide a `conanfile.py` at the root declaring all C/C++ dependencies and configuring the `CMakeToolchain` and `CMakeDeps` generators.

#### Scenario: conan install succeeds on a clean machine
- **WHEN** `conan install . --profile:host=profiles/arm-none-eabi --profile:build=default` is run for the first time
- **THEN** all declared dependencies are downloaded, built for the target, and CMake integration files are generated in `build/<type>/generators/`

### Requirement: arm-none-eabi Conan profile configures cross-compilation
The project SHALL provide a `profiles/arm-none-eabi` Conan profile that sets `os=baremetal`, `arch=armv7hf`, points compiler executables to `arm-none-eabi-gcc`/`arm-none-eabi-g++`, and injects `cmake/arm-none-eabi.cmake` as the CMake user toolchain.

#### Scenario: Conan toolchain includes arm-none-eabi settings
- **WHEN** `conan install` runs with the arm-none-eabi host profile
- **THEN** the generated `conan_toolchain.cmake` includes the CPU flags `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb`

### Requirement: Conan generates CMakeUserPresets.json for VS Code
The Conan install step SHALL generate a `CMakeUserPresets.json` at the project root that VS Code CMake Tools merges with `CMakePresets.json`, exposing `conan-debug` and `conan-release` as selectable presets in the IDE.

#### Scenario: VS Code shows conan presets after install
- **WHEN** `conan install` has been run for both Debug and Release build types
- **THEN** VS Code CMake Tools lists `conan-debug` and `conan-release` as available configure presets

#### Scenario: Selecting conan-debug preset configures correctly
- **WHEN** the `conan-debug` preset is selected and CMake is configured
- **THEN** the build directory is `build/debug`, build type is `Debug`, and the Conan-generated toolchain is active
