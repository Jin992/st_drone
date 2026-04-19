## 1. Repository Init

- [x] 1.1 Run `git init` at project root
- [x] 1.2 Add `.gitignore` covering `build/`, `CMakeUserPresets.json`, `*.elf`, `*.bin`, `*.hex`

## 2. Toolchain File

- [x] 2.1 Create `cmake/arm-none-eabi.cmake` with `CMAKE_SYSTEM_NAME=Generic`, `CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY`, arm-none-eabi compiler paths, and `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb` flags via `CMAKE_C_FLAGS_INIT` and `CMAKE_ASM_FLAGS_INIT`

## 3. Conan Setup

- [x] 3.1 Create `conanfile.py` with `CMakeToolchain` and `CMakeDeps` generators and empty `requires` (ready for future deps)
- [x] 3.2 Create `profiles/arm-none-eabi` Conan profile with `os=baremetal`, `arch=armv7hf`, `compiler.executables` pointing to `arm-none-eabi-gcc`/`g++`, and `user_toolchain` injecting `cmake/arm-none-eabi.cmake`
- [x] 3.3 Run `conan install . --profile:host=profiles/arm-none-eabi --profile:build=default -s build_type=Debug` and verify `conan_toolchain.cmake` and `CMakeUserPresets.json` are generated
- [x] 3.4 Run `conan install` again with `-s build_type=Release` to generate the release preset

## 4. CMakePresets

- [x] 4.1 Create `CMakePresets.json` with a hidden `base` preset (Ninja generator, `CMAKE_EXPORT_COMPILE_COMMANDS=ON`) and `debug` / `release` configure presets inheriting from it with their respective build directories
- [x] 4.2 Add matching `buildPresets` for `debug` and `release`
- [x] 4.3 Verify VS Code CMake Tools lists `conan-debug` and `conan-release` presets after `conan install`

## 5. stm32-cmake Integration

- [x] 5.1 Create root `CMakeLists.txt` with `cmake_minimum_required(VERSION 3.21)`, `project(drone C ASM)`, and `FetchContent_Declare` for stm32-cmake pinned to a release tag
- [x] 5.2 Call `FetchContent_MakeAvailable(stm32-cmake)` and `find_package(STM32 F4 REQUIRED)` in the root CMakeLists
- [x] 5.3 Verify stm32-cmake downloads and configures without errors on first run

## 6. Firmware Target

- [x] 6.1 Create `src/main.c` with HAL init, PC13 GPIO output configuration, and an infinite loop toggling the pin with `HAL_Delay(500)`
- [x] 6.2 Define the `drone` executable target in `CMakeLists.txt`, add `src/main.c`, link `stm32::f405rg` and `stm32::f4::hal`
- [x] 6.3 Apply stm32-cmake linker script helper for STM32F405RGT6
- [x] 6.4 Add post-build commands to generate `firmware.bin` and `firmware.hex` from the ELF via `arm-none-eabi-objcopy`
- [x] 6.5 Build with `cmake --build build/debug` and confirm `drone.elf`, `firmware.bin`, `firmware.hex` are produced

## 7. Subsystem Scaffolding

- [x] 7.1 Create `flight/CMakeLists.txt` defining an empty `flight` static library with a public include directory
- [x] 7.2 Create `sensors/CMakeLists.txt` defining an empty `sensors` static library with a public include directory
- [x] 7.3 Create `comms/CMakeLists.txt` defining an empty `comms` static library with a public include directory
- [x] 7.4 Add `add_subdirectory` calls for all three in root `CMakeLists.txt` and link them into the `drone` target
- [x] 7.5 Confirm full build still succeeds with empty subsystem libraries

## 8. Flash & Debug Targets

- [x] 8.1 Create `openocd.cfg` with `source [find interface/stlink.cfg]` and `source [find target/stm32f4x.cfg]`
- [x] 8.2 Add `flash` custom CMake target invoking `openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $<TARGET_FILE:drone> verify reset exit"`
- [x] 8.3 Add `debug` custom CMake target invoking OpenOCD without the `exit` command to keep the GDB server running
- [x] 8.4 Run `cmake --build build/debug --target flash` with board connected and confirm LED blinks on PC13
