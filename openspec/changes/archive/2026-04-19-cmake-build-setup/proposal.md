## Why

The project needs a clean, reproducible build system as its foundation before any drone logic can be written. The build system must be IDE-friendly (VS Code + CMake Tools), use a package manager for C/C++ dependencies, and be structured to accommodate multiple drone subsystems as the project grows.

## What Changes

- Add `CMakeLists.txt` (root) orchestrating the full firmware build
- Add `CMakePresets.json` with `debug` and `release` configure presets for VS Code CMake Tools
- Add `cmake/arm-none-eabi.cmake` toolchain file for Cortex-M4 + FPU cross-compilation
- Add `conanfile.py` declaring project dependencies and Conan configuration
- Add `profiles/arm-none-eabi` Conan profile for cross-compilation targeting STM32F405RGT6
- Add stm32-cmake (ObKo) via `FetchContent` — provides CMake targets for STM32 HAL, startup files, and linker scripts (not on ConanCenter, so FetchContent is used)
- Add `src/main.c` — minimal LED blink on PC13 proving the full build-to-flash pipeline
- Add `openocd.cfg` and CMake `flash` / `debug` custom targets for STLink v2
- Scaffold `flight/`, `sensors/`, `comms/` subdirectories with stub `CMakeLists.txt` files

## Capabilities

### New Capabilities

- `cmake-toolchain`: Cross-compilation toolchain and CMakePresets.json targeting STM32F405RGT6 (Cortex-M4 + FPU), compatible with VS Code CMake Tools
- `conan-integration`: Conan 2.x package manager with arm-none-eabi cross-compilation profile; generates `conan_toolchain.cmake` and `CMakeUserPresets.json` consumed by VS Code
- `stm32-cmake-integration`: stm32-cmake via FetchContent providing `stm32::f405rg` and `stm32::f4::hal` CMake targets, startup files, and linker script generation
- `flash-debug-targets`: CMake custom targets to flash and debug via OpenOCD + STLink v2
- `blink-firmware`: Minimal `src/main.c` blinking PC13 (blue LED, active-low) as end-to-end build proof

### Modified Capabilities

## Impact

- Requires host tools: `arm-none-eabi-gcc`, `cmake` (3.21+), `openocd` (0.12.0 confirmed), `git`, `conan` (2.x)
- Developer workflow: `conan install` first, then `cmake --preset conan-debug`, then build
- stm32-cmake fetched via FetchContent on first configure (pulls STM32CubeF4 HAL sources ~50 MB)
- Greenfield project — no existing code affected

## Board Notes

- **Board**: WeAct STM32F405RGT6
- **LED**: Blue LED on PC13, active-low
- **User button**: PA0
- **Debugger**: STLink V2J37S7, confirmed working with OpenOCD 0.12.0 via `interface/stlink.cfg` + `target/stm32f4x.cfg`
