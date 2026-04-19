## Context

Greenfield firmware project for WeAct STM32F405RGT6 (Cortex-M4 + FPU, 168 MHz, 1 MB Flash, 192 KB SRAM). No existing build system. Developer uses VS Code with CMake Tools extension and STLink V2J37S7 debugger confirmed working with OpenOCD 0.12.0.

## Goals / Non-Goals

**Goals:**
- Cross-compile with `arm-none-eabi-gcc` via CMake 3.21+
- VS Code CMake Tools selects preset, clicks Build — no CLI flags required
- STM32 HAL available via stm32-cmake (ObKo) — proper CMake targets, no `make` invocations
- Single build produces `.elf`, `.bin`, `.hex`
- `flash` and `debug` CMake targets work out of the box
- Directory structure scales to multiple drone subsystems

**Non-Goals:**
- FreeRTOS or any RTOS
- Drone logic, sensors, motors, ESC
- Windows host support
- CI/CD, unit tests

## Decisions

### D1: Conan 2.x as package manager with arm-none-eabi cross-compilation profile

Conan manages all C/C++ dependencies and generates CMake integration files. The developer workflow is:

```bash
# Step 1 — install deps, generate toolchain + presets
conan install . --profile:host=profiles/arm-none-eabi --profile:build=default -s build_type=Debug

# Step 2 — configure (uses Conan-generated preset)
cmake --preset conan-debug

# Step 3 — build / flash
cmake --build build/debug --target flash
```

Conan 2.x generates two files consumed by the rest of the build:

- `build/debug/generators/conan_toolchain.cmake` — toolchain wiring (includes our `cmake/arm-none-eabi.cmake` via the profile's `user_toolchain` setting)
- `CMakeUserPresets.json` — VS Code CMake Tools merges this with `CMakePresets.json` automatically, exposing `conan-debug` and `conan-release` presets in the IDE

The `profiles/arm-none-eabi` Conan profile:
```ini
[settings]
os=baremetal
arch=armv7hf
compiler=gcc
compiler.version=13

[conf]
tools.build:compiler_executables={'c': 'arm-none-eabi-gcc', 'cpp': 'arm-none-eabi-g++'}
tools.cmake.cmaketoolchain:user_toolchain=["{{os.path.join(profile_dir, '../cmake/arm-none-eabi.cmake')}}"]
```

The `user_toolchain` entry injects our `arm-none-eabi.cmake` into Conan's generated toolchain — CPU flags and bare-metal settings live in one place, not duplicated between Conan and CMake.

stm32-cmake is **not** on ConanCenter, so it remains as FetchContent (see D2). All other future C/C++ dependencies (e.g. COBS, printf, compression libs) go through Conan.

**Alternatives rejected:**
- *FetchContent for everything*: No version resolution, no lockfile, no transitive dependency management.
- *vcpkg*: Weaker cross-compilation story for bare-metal targets; Conan profiles are better suited.

### D2: stm32-cmake (ObKo) via FetchContent for HAL integration

[stm32-cmake](https://github.com/ObKo/stm32-cmake) is a CMake-first wrapper around ST's HAL/LL drivers. It provides proper CMake targets (`stm32::f405rg`, `stm32::f4::hal`) and handles startup files, linker scripts, and HAL source selection automatically. No `make` invocations — the entire dependency graph is native CMake.

```cmake
FetchContent_Declare(stm32-cmake
  GIT_REPOSITORY https://github.com/ObKo/stm32-cmake.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(stm32-cmake)

find_package(STM32 F4 REQUIRED)
target_link_libraries(drone PRIVATE stm32::f405rg stm32::f4::hal)
```

stm32-cmake pulls STM32CubeF4 HAL sources via its own FetchContent — no git submodule needed.

**Alternatives rejected:**
- *libopencm3*: CMake-unfriendly — its build system is `make`-only, requiring `ExternalProject_Add` hacks or manually listing source files.
- *Bare CMSIS*: No peripheral API — would require writing all drivers from scratch.
- *STM32CubeF4 manually*: stm32-cmake wraps it correctly; doing it manually is error-prone.

### D3: CMakePresets.json with hidden base + debug/release presets

A hidden `base` configure preset holds the toolchain path and shared settings. `debug` and `release` inherit from it and set their build directory and optimization flags.

```json
{
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "toolchainFile": "${sourceDir}/cmake/arm-none-eabi.cmake",
      "generator": "Ninja",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "debug",
      "inherits": "base",
      "binaryDir": "${sourceDir}/build/debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "release",
      "inherits": "base",
      "binaryDir": "${sourceDir}/build/release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ]
}
```

`compile_commands.json` is always exported so the C/C++ extension gets accurate IntelliSense for cross-compiler headers.

### D4: Toolchain file handles all cross-compilation setup

`cmake/arm-none-eabi.cmake` is the single place for everything the cross-compiler needs:

- `CMAKE_SYSTEM_NAME Generic` — signals bare-metal, suppresses host linker checks
- `CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY` — prevents CMake's compiler probe from trying to link an executable (fails on bare-metal)
- CPU flags via `CMAKE_C_FLAGS_INIT` / `CMAKE_ASM_FLAGS_INIT`, propagating to all targets:
  `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb`

stm32-cmake also needs the toolchain file — it reads `CMAKE_SYSTEM_PROCESSOR` to select the correct HAL variant.

### D5: Linker script and post-build steps via stm32-cmake helpers

stm32-cmake provides `stm32_add_linker_script()` and generates startup files per target. No manually maintained linker script needed. Post-build `.bin` and `.hex` generation via `stm32_print_size_of_targets()` and `arm-none-eabi-objcopy` commands attached to the firmware target.

### D6: Flash and debug as CMake custom targets

```cmake
add_custom_target(flash
  COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
          -c "program $<TARGET_FILE:drone> verify reset exit"
  DEPENDS drone
)

add_custom_target(debug
  COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
  DEPENDS drone
)
```

`interface/stlink.cfg` confirmed working with V2J37S7 on OpenOCD 0.12.0.

### D7: Subsystem directories scaffold with stub CMakeLists.txt

`flight/`, `sensors/`, `comms/` each get a stub `CMakeLists.txt` defining an empty static library. The root `CMakeLists.txt` links them all into the firmware target. Adding code to a subsystem later requires no changes to the root CMake.

```cmake
# flight/CMakeLists.txt
add_library(flight STATIC)
target_sources(flight PRIVATE)
target_include_directories(flight PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

## Risks / Trade-offs

- **Conan not installed** → Build fails before CMake even runs. Mitigation: document `pip install conan` as a prerequisite; add a check in the README.
- **`conan install` must run before `cmake`** → First-time developer experience requires two steps. Mitigation: document clearly; a wrapper script or VS Code task can chain them.
- **FetchContent pulls STM32CubeF4 on first configure** → First configure downloads ~50 MB HAL sources. Subsequent builds use the CMake cache. Mitigation: document this; CI can cache the FetchContent directory.
- **stm32-cmake `master` branch** → Pinning to a specific commit or tag is safer for reproducibility. Mitigation: pin to a release tag once confirmed working.
- **IntelliSense picks up wrong compiler** → VS Code may default to host GCC. Mitigation: `compile_commands.json` export + `.vscode/c_cpp_properties.json` pointing at it resolves this.
- **Ninja not installed** → Presets default to Ninja. Mitigation: fall back to `"Unix Makefiles"` as an alternative preset; document both.
